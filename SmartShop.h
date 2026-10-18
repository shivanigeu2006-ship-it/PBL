// SmartShop.h - the integration class. Owns every module (composition), loads/saves data
// with C++ file handling, generates smart alerts and exposes JSON-returning operations
// for the web API. All public methods are thread-safe (one recursive mutex).
#pragma once
#include <filesystem>
#include <fstream>
#include <mutex>
#include <map>
#include <set>
#include <cstdint>
#include "Inventory.h"
#include "ShelfManager.h"
#include "SalesRecord.h"
#include "Supplier.h"
#include "DemandAnalyzer.h"
#include "ExpiryManager.h"
#include "ReorderManager.h"
#include "ProfitAnalyzer.h"

struct Alert {
    std::string type, severity, sku, message;  // severity: critical | warning | info
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Alert, type, severity, sku, message)

class SmartShop {
    mutable std::recursive_mutex mtx_;
    std::string dir_;

    Inventory inv_;
    ShelfManager shelves_;
    SalesRecord sales_;
    std::map<std::string, Supplier> suppliers_;
    std::vector<Waste> waste_;

    DemandAnalyzer demand_;
    ExpiryManager expiry_;
    ReorderManager reorder_;
    ProfitAnalyzer profit_;

    static const int WINDOW = 30;     // days of sales used for demand
    static const int SOON_DAYS = 7;   // "expiring soon" horizon

    using Lock = std::lock_guard<std::recursive_mutex>;

    // ------------------------------------------------------------------ validation helpers
    static std::string trim(std::string s) {
        size_t a = s.find_first_not_of(" \t\r\n"), b = s.find_last_not_of(" \t\r\n");
        return a == std::string::npos ? "" : s.substr(a, b - a + 1);
    }
    static std::string reqStr(const json& j, const char* k) {
        if (!j.is_object() || !j.contains(k) || !j[k].is_string()) throw ApiError(400, std::string("'") + k + "' is required");
        std::string v = trim(j[k].get<std::string>());
        if (v.empty()) throw ApiError(400, std::string("'") + k + "' cannot be empty");
        return v;
    }
    static std::string optStr(const json& j, const char* k, const std::string& def = "") {
        if (!j.contains(k) || j[k].is_null()) return def;
        if (!j[k].is_string()) throw ApiError(400, std::string("'") + k + "' must be text");
        return trim(j[k].get<std::string>());
    }
    static double num(const json& j, const char* k, bool required, double def, double minv, bool exclusive = false) {
        if (!j.contains(k) || j[k].is_null()) {
            if (required) throw ApiError(400, std::string("'") + k + "' is required");
            return def;
        }
        if (!j[k].is_number()) throw ApiError(400, std::string("'") + k + "' must be a number");
        double v = j[k].get<double>();
        if (v < minv || (exclusive && v == minv))
            throw ApiError(400, std::string("'") + k + "' must be " + (exclusive ? "greater than " : "at least ") + std::to_string((int)minv));
        return v;
    }
    static int integer(const json& j, const char* k, bool required, int def, int minv) {
        if (!j.contains(k) || j[k].is_null()) {
            if (required) throw ApiError(400, std::string("'") + k + "' is required");
            return def;
        }
        if (!j[k].is_number_integer()) throw ApiError(400, std::string("'") + k + "' must be a whole number");
        int v = j[k].get<int>();
        if (v < minv) throw ApiError(400, std::string("'") + k + "' must be at least " + std::to_string(minv));
        return v;
    }
    static void checkId(const std::string& id, const char* what) {
        if (id.size() > 32) throw ApiError(400, std::string(what) + " is too long (max 32 characters)");
        for (char c : id)
            if (!std::isalnum((unsigned char)c) && c != '-' && c != '_')
                throw ApiError(400, std::string(what) + " may only contain letters, digits, '-' and '_'");
    }
    static std::string money(double v) { char b[32]; std::snprintf(b, sizeof b, "Rs.%.2f", v); return b; }
    static std::string whole(double v) { char b[32]; std::snprintf(b, sizeof b, "%.1f", v); return b; }

    long parseExpiryField(const json& j, long today) const {
        std::string s = optStr(j, "expiry", "");
        long e;
        if (!util::parseDate(s, e)) throw ApiError(400, "'expiry' must be a valid date (yyyy-mm-dd) or empty for non-perishable");
        if (e < today) throw ApiError(400, "'expiry' is in the past - this stock is already expired");
        return e;
    }

    Product& mustFind(const std::string& sku) {
        Product* p = inv_.find(sku);
        if (!p) throw ApiError(404, "Unknown SKU: " + sku);
        return *p;
    }
    const Product& mustFindC(const std::string& sku) const {
        const Product* p = inv_.find(sku);
        if (!p) throw ApiError(404, "Unknown SKU: " + sku);
        return *p;
    }

    // ------------------------------------------------------------------ stock helpers
    // Puts qty units on the product's shelf (auto-picking one if it has none). Capacity is enforced.
    void receiveStock(Product& p, int qty, long expiry, long day) {
        if (p.shelfId().empty() || !shelves_.get(p.shelfId())) {
            std::string pick = shelves_.autoAssign(p, qty);
            if (pick.empty()) {
                if (!shelves_.all().empty())
                    throw ApiError(409, "No shelf has " + std::to_string(qty) + " free spaces. Free up space or add a shelf.");
            } else {
                p.setShelf(pick);
            }
        } else if (!shelves_.get(p.shelfId())->canFit(qty)) {
            const Shelf* s = shelves_.get(p.shelfId());
            throw ApiError(409, "Shelf " + s->name() + " has only " + std::to_string(s->freeSpace()) +
                                    " free space; cannot add " + std::to_string(qty) + " units");
        }
        p.addBatch(qty, expiry, day);
        shelves_.recompute(inv_);
    }

    json productView(const Product& p, long t, const std::map<std::string, ReorderInfo>* ro,
                     const std::map<std::string, DemandInfo>* dm) const {
        json j = p.toJson();
        j.erase("counter");
        std::vector<Batch> bs = p.batches();
        std::stable_sort(bs.begin(), bs.end(), [](const Batch& a, const Batch& b) { return a.expiry < b.expiry; });
        j["batches"] = bs;
        j["stock"] = p.totalStock();
        j["sellable"] = p.sellableStock(t);
        j["expired"] = p.expiredStock(t);
        std::string next;
        for (auto& b : bs) if (b.expiry >= t && b.expiry < util::NO_EXPIRY) { next = util::formatDate(b.expiry); break; }
        j["nextExpiry"] = next;
        j["profitPerUnit"] = util::r2(p.profitPerUnit());
        j["marginPct"] = util::r2(p.marginPct());
        const Shelf* s = shelves_.get(p.shelfId());
        j["shelfName"] = s ? s->name() : "";
        auto sp = suppliers_.find(p.supplierId());
        j["supplierName"] = sp == suppliers_.end() ? "" : sp->second.name();
        if (ro) {
            auto it = ro->find(p.sku());
            if (it != ro->end()) {
                j["status"] = it->second.status;
                j["reorderPoint"] = it->second.reorderPoint;
                j["avgDaily"] = it->second.avgDaily;
                j["orderQty"] = it->second.orderQty;
            }
        }
        if (dm) {
            auto it = dm->find(p.sku());
            if (it != dm->end()) j["demand"] = it->second.level;
        }
        return j;
    }

    // ------------------------------------------------------------------ file handling
    std::string path(const char* f) const { return (std::filesystem::path(dir_) / f).string(); }

    void writeFile(const char* name, const json& j) const {
        std::string tmp = path(name) + ".tmp";
        {
            std::ofstream out(tmp, std::ios::trunc);
            if (!out) throw std::runtime_error("Cannot write " + tmp);
            out << j.dump(2);
        }
        std::filesystem::rename(tmp, path(name));  // atomic-ish swap so a crash never leaves half a file
    }
    static bool readFile(const std::string& p, json& out) {
        std::ifstream in(p);
        if (!in) return false;
        out = json::parse(in);
        return true;
    }

    void save() const {
        std::filesystem::create_directories(dir_);
        json pr = json::array();
        for (auto* p : inv_.all()) pr.push_back(p->toJson());
        writeFile("products.json", pr);
        json sh = json::array();
        for (auto& s : shelves_.all()) sh.push_back(s.toJson());
        writeFile("shelves.json", sh);
        json su = json::array();
        for (auto& kv : suppliers_) su.push_back(kv.second.toJson());
        writeFile("suppliers.json", su);
        writeFile("sales.json", {{"counter", sales_.counter()}, {"sales", sales_.all()}});
        writeFile("wastage.json", waste_);
    }

    bool load() {
        json j;
        if (!readFile(path("products.json"), j)) return false;
        inv_.clear(); shelves_.clear(); suppliers_.clear(); sales_.clear(); waste_.clear();
        for (auto& x : j) inv_.add(Product::fromJson(x));
        if (readFile(path("shelves.json"), j)) for (auto& x : j) shelves_.add(Shelf::fromJson(x));
        if (readFile(path("suppliers.json"), j)) for (auto& x : j) { Supplier s = Supplier::fromJson(x); suppliers_[s.id()] = s; }
        if (readFile(path("sales.json"), j)) sales_.load(j.value("sales", std::vector<Sale>{}), j.value("counter", 0));
        if (readFile(path("wastage.json"), j)) waste_ = j.get<std::vector<Waste>>();
        shelves_.recompute(inv_);
        return true;
    }

public:
    explicit SmartShop(const std::string& dataDir) : dir_(dataDir) {
        if (!load()) { seedDemo(); }
    }

    // ================================================================== demo data
    void seedDemo() {
        Lock g(mtx_);
        inv_.clear(); shelves_.clear(); suppliers_.clear(); sales_.clear(); waste_.clear();
        const long T = util::today();

        suppliers_["SUP1"] = Supplier("SUP1", "FreshDairy Co.", "98100 11111", 1, 10);
        suppliers_["SUP2"] = Supplier("SUP2", "Daily Wholesale Mart", "98100 22222", 3, 15);
        suppliers_["SUP3"] = Supplier("SUP3", "HomeCare Distributors", "98100 33333", 5, 8);
        suppliers_["SUP4"] = Supplier("SUP4", "Baker's Choice", "98100 44444", 1, 6);

        shelves_.add(Shelf("SH-A1", "Chiller 1", "Dairy", 150));
        shelves_.add(Shelf("SH-A2", "Bakery Rack", "Bakery", 80));
        shelves_.add(Shelf("SH-B1", "Aisle B - Grocery", "Grocery", 300));
        shelves_.add(Shelf("SH-B2", "Aisle B - Snacks", "Snacks", 300));
        shelves_.add(Shelf("SH-C1", "Aisle C - Beverages", "Beverages", 250));
        shelves_.add(Shelf("SH-D1", "Household Wall", "Household", 120));
        shelves_.add(Shelf("SH-E1", "Counter Display", "Personal Care", 60));
        shelves_.add(Shelf("SH-G1", "Overflow Rack", "General", 150));

        struct SB { int qty; int exp; };  // exp = days from today (>= 9000 means no expiry)
        struct SP {
            const char *sku, *name, *cat; double cost, price; int target; const char *sup, *shelf; double rate;
            std::vector<SB> b;
        };
        const std::vector<SP> seed = {
            {"MILK-1L", "Toned Milk 1L", "Dairy", 52, 56, 80, "SUP1", "SH-A1", 22, {{14, 1}, {12, 3}}},
            {"CURD-400", "Fresh Curd 400g", "Dairy", 28, 35, 40, "SUP1", "SH-A1", 9, {{10, 2}, {14, 6}}},
            {"YOG-STR", "Strawberry Yogurt Cup", "Dairy", 14, 20, 30, "SUP1", "SH-A1", 1.2, {{9, -2}, {12, 4}}},
            {"PANEER-200", "Paneer 200g", "Dairy", 70, 90, 25, "SUP1", "SH-A1", 6, {{5, 2}, {8, 9}}},
            {"BUTTER-100", "Butter 100g", "Dairy", 50, 58, 30, "SUP1", "SH-A1", 3, {{35, 60}}},
            {"BREAD-WH", "White Bread", "Bakery", 28, 40, 40, "SUP4", "SH-A2", 14, {{6, 0}, {22, 2}}},
            {"PAV-6", "Pav Buns (6)", "Bakery", 20, 30, 20, "SUP4", "SH-A2", 5, {{8, 1}}},
            {"RICE-5KG", "Basmati Rice 5kg", "Grocery", 380, 450, 40, "SUP2", "SH-B1", 3.5, {{70, 300}}},
            {"ATTA-5KG", "Wheat Flour 5kg", "Grocery", 210, 245, 40, "SUP2", "SH-B1", 6, {{30, 120}}},
            {"SUGAR-1KG", "Sugar 1kg", "Grocery", 40, 48, 60, "SUP2", "SH-B1", 8, {{70, 400}}},
            {"OIL-1L", "Sunflower Oil 1L", "Grocery", 120, 145, 40, "SUP2", "SH-B1", 5, {{45, 150}}},
            {"DAL-1KG", "Toor Dal 1kg", "Grocery", 130, 160, 30, "SUP2", "SH-B1", 2, {{12, 200}}},
            {"CHIPS-MS", "Masala Chips 90g", "Snacks", 15, 20, 100, "SUP2", "SH-B2", 18, {{60, 90}, {40, 30}}},
            {"BISC-GL", "Glucose Biscuits", "Snacks", 8, 10, 120, "SUP2", "SH-B2", 25, {{30, 45}}},
            {"NOODLE-1", "Instant Noodles", "Snacks", 12, 15, 80, "SUP2", "SH-B2", 12, {{85, 120}}},
            {"CHOC-DM", "Milk Chocolate 50g", "Snacks", 38, 45, 40, "SUP2", "SH-B2", 0.5, {{70, 25}}},
            {"COLA-750", "Cola 750ml", "Beverages", 30, 40, 80, "SUP2", "SH-C1", 10, {{50, 60}, {40, 150}}},
            {"JUICE-1L", "Mango Juice 1L", "Beverages", 70, 90, 30, "SUP2", "SH-C1", 2.5, {{12, 5}, {10, 40}}},
            {"WATER-1L", "Packaged Water 1L", "Beverages", 12, 20, 120, "SUP2", "SH-C1", 20, {{100, 300}}},
            {"DET-BAR", "Detergent Bar", "Household", 12, 18, 60, "SUP3", "SH-D1", 4, {{80, 9999}}},
            {"DISH-500", "Dishwash Liquid 500ml", "Household", 85, 110, 25, "SUP3", "SH-D1", 1.5, {}},
            {"TOOTH-100", "Toothpaste 100g", "Personal Care", 55, 75, 30, "SUP3", "SH-E1", 3, {{28, 9999}}},
        };

        uint32_t seedv = 20260101u;
        auto rnd = [&]() { seedv = seedv * 1664525u + 1013904223u; return (seedv >> 8) / 16777216.0; };
        std::vector<Sale> hist;
        int sid = 0;
        for (auto& s : seed) {
            Product p(s.sku, s.name, s.cat, s.cost, s.price, s.target);
            p.setShelf(s.shelf);
            p.setSupplier(s.sup);
            for (auto& b : s.b) p.addBatch(b.qty, b.exp >= 9000 ? util::NO_EXPIRY : T + b.exp, T - 10);
            inv_.add(p);
            for (int d = -29; d <= 0; ++d) {  // 30 days of sales history
                int units = (int)(s.rate * (0.5 + rnd()) + rnd());
                if (units <= 0) continue;
                Sale sl;
                char buf[16];
                std::snprintf(buf, sizeof buf, "S%05d", ++sid);
                sl.id = buf; sl.sku = s.sku; sl.name = s.name; sl.units = units; sl.day = T + d;
                sl.unitPrice = s.price; sl.unitCost = s.cost;
                hist.push_back(sl);
            }
        }
        std::stable_sort(hist.begin(), hist.end(), [](const Sale& a, const Sale& b) { return a.day < b.day; });
        sales_.load(hist, sid);
        waste_.push_back({"BREAD-WH", "White Bread", "BREAD-WH-B000", util::formatDate(T - 10), 4, 112.0});
        shelves_.recompute(inv_);
        save();
    }

    // ================================================================== products
    json listProducts(const std::string& q) const {
        Lock g(mtx_);
        long t = util::today();
        std::map<std::string, ReorderInfo> ro;
        for (auto& r : reorder_.analyze(inv_, sales_, suppliers_, shelves_, demand_, t, WINDOW)) ro[r.sku] = r;
        std::map<std::string, DemandInfo> dm;
        for (auto& d : demand_.analyze(inv_, sales_, t, WINDOW)) dm[d.sku] = d;
        auto low = [](std::string s) { for (auto& c : s) c = (char)std::tolower((unsigned char)c); return s; };
        std::string needle = low(q);
        json arr = json::array();
        for (auto* p : inv_.all()) {
            if (!needle.empty() && low(p->sku()).find(needle) == std::string::npos &&
                low(p->name()).find(needle) == std::string::npos && low(p->category()).find(needle) == std::string::npos)
                continue;
            arr.push_back(productView(*p, t, &ro, &dm));
        }
        return arr;
    }

    json getProduct(const std::string& sku) const {
        Lock g(mtx_);
        long t = util::today();
        const Product& p = mustFindC(sku);
        std::map<std::string, ReorderInfo> ro;
        for (auto& r : reorder_.analyze(inv_, sales_, suppliers_, shelves_, demand_, t, WINDOW)) ro[r.sku] = r;
        std::map<std::string, DemandInfo> dm;
        for (auto& d : demand_.analyze(inv_, sales_, t, WINDOW)) dm[d.sku] = d;
        return productView(p, t, &ro, &dm);
    }

    json addProduct(const json& b) {
        Lock g(mtx_);
        long t = util::today();
        std::string sku = reqStr(b, "sku");
        checkId(sku, "SKU");
        if (inv_.exists(sku)) throw ApiError(409, "SKU " + sku + " already exists");
        Product p(sku, reqStr(b, "name"), optStr(b, "category", "General"), num(b, "cost", true, 0, 0),
                  num(b, "price", true, 0, 0, true), integer(b, "target", true, 0, 1));
        int safety = integer(b, "safety", false, -1, -1);
        p.setSafety(safety);
        std::string sup = optStr(b, "supplierId");
        if (!sup.empty() && !suppliers_.count(sup)) throw ApiError(400, "Unknown supplier: " + sup);
        p.setSupplier(sup);
        std::string shelf = optStr(b, "shelfId");
        if (!shelf.empty() && !shelves_.get(shelf)) throw ApiError(400, "Unknown shelf: " + shelf);
        p.setShelf(shelf);
        int qty = integer(b, "initialQty", false, 0, 0);
        long exp = util::NO_EXPIRY;
        if (qty > 0) {
            exp = parseExpiryField(b, t);
            receiveStock(p, qty, exp, t);  // capacity check happens before the product is stored
        } else if (shelf.empty()) {
            p.setShelf(shelves_.autoAssign(p, 0));
        }
        inv_.add(p);
        shelves_.recompute(inv_);
        save();
        return getProduct(sku);
    }

    json updateProduct(const std::string& sku, const json& b) {
        Lock g(mtx_);
        Product& p = mustFind(sku);
        if (b.contains("name")) p.setName(reqStr(b, "name"));
        if (b.contains("category")) p.setCategory(optStr(b, "category", "General"));
        if (b.contains("cost")) p.setCost(num(b, "cost", true, 0, 0));
        if (b.contains("price")) p.setPrice(num(b, "price", true, 0, 0, true));
        if (b.contains("target")) p.setTarget(integer(b, "target", true, 0, 1));
        if (b.contains("safety")) p.setSafety(integer(b, "safety", false, -1, -1));
        if (b.contains("supplierId")) {
            std::string sup = optStr(b, "supplierId");
            if (!sup.empty() && !suppliers_.count(sup)) throw ApiError(400, "Unknown supplier: " + sup);
            p.setSupplier(sup);
        }
        if (b.contains("shelfId")) {
            std::string sh = optStr(b, "shelfId"), err;
            if (sh.empty()) p.setShelf("");
            else if (!shelves_.assign(inv_, sku, sh, err)) throw ApiError(409, err);
        }
        shelves_.recompute(inv_);
        save();
        return getProduct(sku);
    }

    void deleteProduct(const std::string& sku) {
        Lock g(mtx_);
        if (!inv_.remove(sku)) throw ApiError(404, "Unknown SKU: " + sku);
        shelves_.recompute(inv_);
        save();
    }

    json addBatch(const std::string& sku, const json& b) {
        Lock g(mtx_);
        long t = util::today();
        Product& p = mustFind(sku);
        int qty = integer(b, "qty", true, 0, 1);
        long exp = parseExpiryField(b, t);
        std::string sh = optStr(b, "shelfId");
        if (!sh.empty() && sh != p.shelfId()) {
            if (!shelves_.get(sh)) throw ApiError(400, "Unknown shelf: " + sh);
            if (p.totalStock() == 0) p.setShelf(sh);  // empty product can move freely
        }
        receiveStock(p, qty, exp, t);
        save();
        return getProduct(sku);
    }

    // ================================================================== sales
    json recordSale(const json& b) {
        Lock g(mtx_);
        long t = util::today(), day = t;
        std::string sku = reqStr(b, "sku");
        int units = integer(b, "units", true, 0, 1);
        std::string d = optStr(b, "date");
        if (!d.empty()) {
            if (!util::parseDate(d, day) || day == util::NO_EXPIRY) throw ApiError(400, "'date' must be yyyy-mm-dd");
            if (day > t) throw ApiError(400, "'date' cannot be in the future");
        }
        if (!inv_.exists(sku)) throw ApiError(404, "Unknown SKU: " + sku);
        auto r = sales_.record(inv_, sku, units, day);
        if (!r.ok) throw ApiError(409, r.error);
        shelves_.recompute(inv_);
        save();
        json out = {{"sale", r.sale}, {"product", getProduct(sku)}};
        return out;
    }

    json listSales(const std::string& sku, int limit) const {
        Lock g(mtx_);
        json arr = json::array();
        auto& all = sales_.all();
        for (auto it = all.rbegin(); it != all.rend() && (int)arr.size() < limit; ++it)
            if (sku.empty() || it->sku == sku) arr.push_back(*it);
        return arr;
    }

    // ================================================================== suppliers
    json listSuppliers() const {
        Lock g(mtx_);
        json arr = json::array();
        for (auto& kv : suppliers_) {
            json j = kv.second.toJson();
            int n = 0;
            for (auto* p : inv_.all()) if (p->supplierId() == kv.first) ++n;
            j["products"] = n;
            arr.push_back(j);
        }
        return arr;
    }
    json addSupplier(const json& b) {
        Lock g(mtx_);
        std::string id = reqStr(b, "id");
        checkId(id, "Supplier id");
        if (suppliers_.count(id)) throw ApiError(409, "Supplier " + id + " already exists");
        Supplier s(id, reqStr(b, "name"), optStr(b, "phone"), integer(b, "leadTime", true, 1, 0),
                   integer(b, "safetyStock", false, 0, 0));
        suppliers_[id] = s;
        save();
        return s.toJson();
    }
    json updateSupplier(const std::string& id, const json& b) {
        Lock g(mtx_);
        auto it = suppliers_.find(id);
        if (it == suppliers_.end()) throw ApiError(404, "Unknown supplier: " + id);
        Supplier& s = it->second;
        if (b.contains("name")) s.setName(reqStr(b, "name"));
        if (b.contains("phone")) s.setPhone(optStr(b, "phone"));
        if (b.contains("leadTime")) s.setLeadTime(integer(b, "leadTime", true, 1, 0));
        if (b.contains("safetyStock")) s.setSafetyStock(integer(b, "safetyStock", true, 0, 0));
        save();
        return s.toJson();
    }
    void deleteSupplier(const std::string& id) {
        Lock g(mtx_);
        if (!suppliers_.count(id)) throw ApiError(404, "Unknown supplier: " + id);
        for (auto* p : inv_.all())
            if (p->supplierId() == id) throw ApiError(409, "Supplier is used by " + p->name() + " - reassign its products first");
        suppliers_.erase(id);
        save();
    }

    // ================================================================== shelves
    json shelvesView() const {
        Lock g(mtx_);
        long t = util::today();
        json arr = json::array();
        for (auto& s : shelves_.all()) {
            json o = s.toJson();
            o["used"] = s.used();
            o["free"] = s.freeSpace();
            o["usagePct"] = util::r2(s.usagePct());
            json items = json::array();
            for (auto* p : inv_.all())
                if (p->shelfId() == s.id())
                    items.push_back({{"sku", p->sku()}, {"name", p->name()}, {"stock", p->totalStock()},
                                     {"expired", p->expiredStock(t)}});
            o["products"] = items;
            arr.push_back(o);
        }
        json un = json::array();
        for (auto* p : inv_.all())
            if (p->shelfId().empty() || !shelves_.get(p->shelfId()))
                un.push_back({{"sku", p->sku()}, {"name", p->name()}, {"stock", p->totalStock()}});
        return {{"shelves", arr}, {"unassigned", un}, {"suggestions", shelves_.suggestions(inv_)}};
    }
    json addShelf(const json& b) {
        Lock g(mtx_);
        std::string id = reqStr(b, "id");
        checkId(id, "Shelf id");
        if (shelves_.get(id)) throw ApiError(409, "Shelf " + id + " already exists");
        Shelf s(id, reqStr(b, "name"), optStr(b, "zone", "General"), integer(b, "capacity", true, 0, 1));
        shelves_.add(s);
        shelves_.recompute(inv_);
        save();
        return shelvesView();
    }
    json updateShelf(const std::string& id, const json& b) {
        Lock g(mtx_);
        Shelf* s = shelves_.get(id);
        if (!s) throw ApiError(404, "Unknown shelf: " + id);
        if (b.contains("name")) s->setName(reqStr(b, "name"));
        if (b.contains("zone")) s->setZone(optStr(b, "zone", "General"));
        if (b.contains("capacity")) {
            int c = integer(b, "capacity", true, 0, 1);
            if (c < s->used()) throw ApiError(409, "Capacity cannot be below the " + std::to_string(s->used()) + " units already on this shelf");
            s->setCapacity(c);
        }
        save();
        return shelvesView();
    }
    void deleteShelf(const std::string& id) {
        Lock g(mtx_);
        if (!shelves_.get(id)) throw ApiError(404, "Unknown shelf: " + id);
        for (auto* p : inv_.all())
            if (p->shelfId() == id) throw ApiError(409, "Shelf still holds " + p->name() + " - move it first");
        shelves_.remove(id);
        save();
    }
    json assignShelf(const json& b) {
        Lock g(mtx_);
        std::string err, sku = reqStr(b, "sku"), sh = reqStr(b, "shelfId");
        if (!inv_.exists(sku)) throw ApiError(404, "Unknown SKU: " + sku);
        if (!shelves_.assign(inv_, sku, sh, err)) throw ApiError(409, err);
        save();
        return shelvesView();
    }
    json autoAssignAll() {
        Lock g(mtx_);
        json done = json::array();
        for (auto* p : inv_.allMutable()) {
            if (!p->shelfId().empty() && shelves_.get(p->shelfId())) continue;
            std::string pick = shelves_.autoAssign(*p, p->totalStock());
            if (pick.empty()) continue;
            p->setShelf(pick);
            shelves_.recompute(inv_);
            done.push_back({{"sku", p->sku()}, {"shelfId", pick}});
        }
        save();
        return {{"assigned", done}, {"view", shelvesView()}};
    }

    // ================================================================== analysis
    json demandView(int days) const {
        Lock g(mtx_);
        return demand_.analyze(inv_, sales_, util::today(), days);
    }
    json expiryView(int soon) const {
        Lock g(mtx_);
        if (soon < 0) soon = SOON_DAYS;
        auto q = expiry_.fefoQueue(inv_, util::today(), soon);
        double risk = 0; int expiredUnits = 0, soonUnits = 0;
        for (auto& e : q) {
            if (e.status == "expired") { risk += e.valueAtRisk; expiredUnits += e.qty; }
            else if (e.status == "critical" || e.status == "soon") soonUnits += e.qty;
        }
        return {{"items", q}, {"soonDays", soon}, {"expiredUnits", expiredUnits}, {"expiringUnits", soonUnits},
                {"expiredValue", util::r2(risk)}};
    }
    json discardExpired(const std::string& sku) {
        Lock g(mtx_);
        long t = util::today();
        int units = 0; double loss = 0;
        json removed = json::array();
        for (auto* p : inv_.allMutable()) {
            if (!sku.empty() && p->sku() != sku) continue;
            for (auto& b : p->discardExpired(t)) {
                Waste w{p->sku(), p->name(), b.id, util::formatDate(t), b.qty, util::r2(b.qty * p->cost())};
                waste_.push_back(w);
                units += b.qty; loss += w.cost;
                removed.push_back(w);
            }
        }
        if (!sku.empty() && !inv_.exists(sku)) throw ApiError(404, "Unknown SKU: " + sku);
        shelves_.recompute(inv_);
        save();
        return {{"removed", removed}, {"units", units}, {"loss", util::r2(loss)}};
    }
    json reorderView(int days) const {
        Lock g(mtx_);
        auto all = reorder_.analyze(inv_, sales_, suppliers_, shelves_, demand_, util::today(), days);
        double cost = 0; int need = 0;
        for (auto& r : all) if (r.orderQty > 0) { cost += r.estCost; ++need; }
        std::sort(all.begin(), all.end(), [](const ReorderInfo& a, const ReorderInfo& b) {
            auto w = [](const std::string& s) { return s == "out_of_stock" ? 0 : s == "reorder" ? 1 : s == "overstock" ? 2 : 3; };
            return w(a.status) < w(b.status);
        });
        return {{"items", all}, {"needsReorder", need}, {"estimatedCost", util::r2(cost)}};
    }
    json profitView() const {
        Lock g(mtx_);
        return {{"summary", profit_.summary(sales_, waste_)}, {"products", profit_.perProduct(inv_, sales_)}};
    }
    json wastageView() const {
        Lock g(mtx_);
        return waste_;
    }

    // ================================================================== smart alerts
    std::vector<Alert> alerts() const {
        Lock g(mtx_);
        long t = util::today();
        std::vector<Alert> a;
        for (auto& e : expiry_.fefoQueue(inv_, t, SOON_DAYS)) {
            std::string unit = std::to_string(e.qty) + " unit(s) of " + e.name + " (" + e.batchId + ")";
            if (e.status == "expired")
                a.push_back({"EXPIRED", "critical", e.sku, unit + " expired on " + e.expiry + ". Remove from the shelf (cost " + money(e.valueAtRisk) + ")."});
            else if (e.status == "critical")
                a.push_back({"EXPIRY_CRITICAL", "critical", e.sku, unit + (e.daysLeft == 0 ? " expire today" : " expire in " + std::to_string(e.daysLeft) + " day(s)") + ". Sell these first (FEFO) or mark down."});
            else if (e.status == "soon")
                a.push_back({"EXPIRY_SOON", "warning", e.sku, unit + " expire in " + std::to_string(e.daysLeft) + " days (" + e.expiry + ")."});
        }
        for (auto& r : reorder_.analyze(inv_, sales_, suppliers_, shelves_, demand_, t, WINDOW)) {
            std::string from = r.supplierName.empty() ? "" : " from " + r.supplierName + " (lead time " + std::to_string(r.leadTime) + " day(s))";
            if (r.status == "out_of_stock")
                a.push_back({"OUT_OF_STOCK", "critical", r.sku, r.name + " is out of stock. Order " + std::to_string(r.orderQty) + " units" + from + "."});
            else if (r.status == "reorder")
                a.push_back({"REORDER", "warning", r.sku, r.name + " has " + std::to_string(r.current) + " units, at or below its reorder point of " + whole(r.reorderPoint) + ". Order " + std::to_string(r.orderQty) + " units" + from + "."});
            else if (r.status == "overstock")
                a.push_back({"OVERSTOCK", "info", r.sku, r.name + " has " + std::to_string(r.current) + " units against a target of " + std::to_string(r.target) + ". Hold off ordering or run a promotion."});
            if (r.spaceWarning)
                a.push_back({"NO_SPACE", "warning", r.sku, "Suggested order of " + std::to_string(r.orderQty) + " " + r.name + " will not fit: its shelf has " + std::to_string(std::max(0, r.shelfFree)) + " free space."});
        }
        for (auto& s : shelves_.all())
            if (s.usagePct() >= 90.0)
                a.push_back({"SHELF_FULL", "warning", "", s.name() + " is " + whole(s.usagePct()) + "% full (" + std::to_string(s.freeSpace()) + " spaces left). See rearrangement suggestions."});
        for (auto* p : inv_.all())
            if (p->shelfId().empty() || !shelves_.get(p->shelfId()))
                a.push_back({"NO_SHELF", "info", p->sku(), p->name() + " is not assigned to a shelf."});
        auto rank = [](const std::string& s) { return s == "critical" ? 0 : s == "warning" ? 1 : 2; };
        std::stable_sort(a.begin(), a.end(), [&](const Alert& x, const Alert& y) { return rank(x.severity) < rank(y.severity); });
        return a;
    }

    json dashboard() const {
        Lock g(mtx_);
        long t = util::today();
        auto ro = reorder_.analyze(inv_, sales_, suppliers_, shelves_, demand_, t, WINDOW);
        int low = 0, out = 0, over = 0;
        for (auto& r : ro) { if (r.status == "reorder") ++low; if (r.status == "out_of_stock") ++out; if (r.status == "overstock") ++over; }
        double stockCost = 0, stockRetail = 0;
        for (auto* p : inv_.all()) { stockCost += p->cost() * p->sellableStock(t); stockRetail += p->price() * p->sellableStock(t); }
        auto ex = expiry_.fefoQueue(inv_, t, SOON_DAYS);
        int expiredUnits = 0, soonBatches = 0;
        for (auto& e : ex) { if (e.status == "expired") expiredUnits += e.qty; else if (e.status != "ok") ++soonBatches; }
        auto al = alerts();
        int crit = 0, warn = 0, info = 0;
        for (auto& x : al) { if (x.severity == "critical") ++crit; else if (x.severity == "warning") ++warn; else ++info; }
        json daily = json::array();
        for (int d = 13; d >= 0; --d) {
            long day = t - d;
            int units = 0; double rev = 0, prof = 0;
            for (auto& s : sales_.all()) if (s.day == day) { units += s.units; rev += s.revenue(); prof += s.profit(); }
            daily.push_back({{"date", util::formatDate(day)}, {"units", units}, {"revenue", util::r2(rev)}, {"profit", util::r2(prof)}});
        }
        json top = json::array();
        auto dm = demand_.analyze(inv_, sales_, t, WINDOW);
        for (size_t i = 0; i < dm.size() && i < 5; ++i) top.push_back(dm[i]);
        json firstAlerts = json::array();
        for (size_t i = 0; i < al.size() && i < 6; ++i) firstAlerts.push_back(al[i]);
        double usedSum = 0, capSum = 0;
        for (auto& s : shelves_.all()) { usedSum += s.used(); capSum += s.capacity(); }
        return {{"today", util::formatDate(t)},
                {"products", (int)inv_.size()}, {"units", inv_.totalUnits()},
                {"stockCost", util::r2(stockCost)}, {"stockRetail", util::r2(stockRetail)},
                {"lowStock", low}, {"outOfStock", out}, {"overstock", over},
                {"expiredUnits", expiredUnits}, {"expiringBatches", soonBatches},
                {"alertCounts", {{"critical", crit}, {"warning", warn}, {"info", info}}},
                {"shelfUsagePct", capSum > 0 ? util::r2(100.0 * usedSum / capSum) : 0.0},
                {"profit", profit_.summary(sales_, waste_)},
                {"daily", daily}, {"topDemand", top}, {"alerts", firstAlerts}};
    }
};
