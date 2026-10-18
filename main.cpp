// main.cpp - SmartStock web server.
// Serves the browser UI (./frontend) and a JSON REST API backed by the SmartShop class.
//   Usage: smartstock [--port 8080] [--data data] [--web frontend]
#include <iostream>
#include <functional>
#include <filesystem>
#include <sstream>
#include <algorithm>
#include "httplib.h"
#include "SmartShop.h"

namespace {

json parseBody(const httplib::Request& req) {
    if (req.body.empty()) return json::object();
    json j = json::parse(req.body, nullptr, false);
    if (j.is_discarded() || !j.is_object()) throw ApiError(400, "Request body must be a JSON object");
    return j;
}

int queryInt(const httplib::Request& req, const char* key, int def) {
    if (!req.has_param(key)) return def;
    try { return std::stoi(req.get_param_value(key)); }
    catch (...) { throw ApiError(400, std::string("Query parameter '") + key + "' must be a number"); }
}

std::string csvCell(const json& v) {
    std::string s = v.is_string() ? v.get<std::string>() : (v.is_null() ? "" : (v.is_object() || v.is_array()) ? "" : v.dump());
    if (s.find_first_of(",\"\n") != std::string::npos) {
        std::string q = "\"";
        for (char c : s) { if (c == '"') q += '"'; q += c; }
        s = q + "\"";
    }
    return s;
}

std::string toCsv(const json& rows) {
    if (!rows.is_array() || rows.empty()) return "";
    std::vector<std::string> cols;
    for (auto it = rows[0].begin(); it != rows[0].end(); ++it)
        if (!it.value().is_object() && !it.value().is_array()) cols.push_back(it.key());
    for (const char* lead : {"name", "sku"}) {  // readable column order: sku, name, then the rest
        auto it = std::find(cols.begin(), cols.end(), lead);
        if (it != cols.end()) { cols.erase(it); cols.insert(cols.begin(), lead); }
    }
    std::ostringstream out;
    for (size_t i = 0; i < cols.size(); ++i) out << (i ? "," : "") << cols[i];
    out << "\n";
    for (auto& r : rows) {
        for (size_t i = 0; i < cols.size(); ++i) out << (i ? "," : "") << csvCell(r.value(cols[i], json()));
        out << "\n";
    }
    return out.str();
}

std::string findWebDir(const std::string& hint) {
    namespace fs = std::filesystem;
    for (std::string c : {hint, std::string("frontend"), std::string("../frontend")})
        if (fs::exists(fs::path(c) / "index.html")) return c;
    return hint;
}

}  // namespace

int main(int argc, char** argv) {
    int port = 8080;
    std::string dataDir = "data", webDir = "frontend";
    for (int i = 1; i < argc; ++i) {
        std::string a = argv[i];
        if (a == "--port" && i + 1 < argc) port = std::atoi(argv[++i]);
        else if (a == "--data" && i + 1 < argc) dataDir = argv[++i];
        else if (a == "--web" && i + 1 < argc) webDir = argv[++i];
        else if (a == "--help") { std::cout << "smartstock [--port 8080] [--data data] [--web frontend]\n"; return 0; }
    }
    webDir = findWebDir(webDir);

    SmartShop shop(dataDir);
    httplib::Server svr;

    // Wraps a handler: JSON in/out, ApiError -> proper HTTP status, anything else -> 500.
    using Handler = std::function<json(const httplib::Request&, httplib::Response&)>;
    auto wrap = [](Handler fn) {
        return [fn](const httplib::Request& req, httplib::Response& res) {
            try {
                json out = fn(req, res);
                res.set_content(out.dump(), "application/json");
            } catch (const ApiError& e) {
                res.status = e.status;
                res.set_content(json({{"error", e.what()}}).dump(), "application/json");
            } catch (const json::exception& e) {
                res.status = 400;
                res.set_content(json({{"error", std::string("Invalid data: ") + e.what()}}).dump(), "application/json");
            } catch (const std::exception& e) {
                res.status = 500;
                res.set_content(json({{"error", std::string("Server error: ") + e.what()}}).dump(), "application/json");
            }
        };
    };
    auto created = [](httplib::Response& res) { res.status = 201; };

    // ---- dashboard, alerts
    svr.Get("/api/health", wrap([&](auto&, auto&) { return json({{"status", "ok"}}); }));
    svr.Get("/api/dashboard", wrap([&](auto&, auto&) { return shop.dashboard(); }));
    svr.Get("/api/alerts", wrap([&](auto&, auto&) { return json(shop.alerts()); }));

    // ---- products
    svr.Get("/api/products", wrap([&](auto& req, auto&) { return shop.listProducts(req.get_param_value("q")); }));
    svr.Post("/api/products", wrap([&](auto& req, auto& res) { json r = shop.addProduct(parseBody(req)); created(res); return r; }));
    svr.Get("/api/products/:sku", wrap([&](auto& req, auto&) { return shop.getProduct(req.path_params.at("sku")); }));
    svr.Put("/api/products/:sku", wrap([&](auto& req, auto&) { return shop.updateProduct(req.path_params.at("sku"), parseBody(req)); }));
    svr.Delete("/api/products/:sku", wrap([&](auto& req, auto&) { shop.deleteProduct(req.path_params.at("sku")); return json({{"deleted", true}}); }));
    svr.Post("/api/products/:sku/batches", wrap([&](auto& req, auto& res) { json r = shop.addBatch(req.path_params.at("sku"), parseBody(req)); created(res); return r; }));

    // ---- sales
    svr.Post("/api/sales", wrap([&](auto& req, auto& res) { json r = shop.recordSale(parseBody(req)); created(res); return r; }));
    svr.Get("/api/sales", wrap([&](auto& req, auto&) { return shop.listSales(req.get_param_value("sku"), queryInt(req, "limit", 100)); }));

    // ---- suppliers
    svr.Get("/api/suppliers", wrap([&](auto&, auto&) { return shop.listSuppliers(); }));
    svr.Post("/api/suppliers", wrap([&](auto& req, auto& res) { json r = shop.addSupplier(parseBody(req)); created(res); return r; }));
    svr.Put("/api/suppliers/:id", wrap([&](auto& req, auto&) { return shop.updateSupplier(req.path_params.at("id"), parseBody(req)); }));
    svr.Delete("/api/suppliers/:id", wrap([&](auto& req, auto&) { shop.deleteSupplier(req.path_params.at("id")); return json({{"deleted", true}}); }));

    // ---- shelves
    svr.Get("/api/shelves", wrap([&](auto&, auto&) { return shop.shelvesView(); }));
    svr.Post("/api/shelves", wrap([&](auto& req, auto& res) { json r = shop.addShelf(parseBody(req)); created(res); return r; }));
    svr.Post("/api/shelves/assign", wrap([&](auto& req, auto&) { return shop.assignShelf(parseBody(req)); }));
    svr.Post("/api/shelves/auto-assign", wrap([&](auto&, auto&) { return shop.autoAssignAll(); }));
    svr.Put("/api/shelves/:id", wrap([&](auto& req, auto&) { return shop.updateShelf(req.path_params.at("id"), parseBody(req)); }));
    svr.Delete("/api/shelves/:id", wrap([&](auto& req, auto&) { shop.deleteShelf(req.path_params.at("id")); return json({{"deleted", true}}); }));

    // ---- analysis
    svr.Get("/api/demand", wrap([&](auto& req, auto&) { return shop.demandView(queryInt(req, "days", 30)); }));
    svr.Get("/api/expiry", wrap([&](auto& req, auto&) { return shop.expiryView(queryInt(req, "days", 7)); }));
    svr.Post("/api/expiry/discard", wrap([&](auto& req, auto&) { return shop.discardExpired(parseBody(req).value("sku", "")); }));
    svr.Get("/api/reorder", wrap([&](auto& req, auto&) { return shop.reorderView(queryInt(req, "days", 30)); }));
    svr.Get("/api/profit", wrap([&](auto&, auto&) { return shop.profitView(); }));
    svr.Get("/api/wastage", wrap([&](auto&, auto&) { return shop.wastageView(); }));

    // ---- admin
    svr.Post("/api/seed", wrap([&](auto& req, auto&) {
        if (!parseBody(req).value("confirm", false)) throw ApiError(400, "Send {\"confirm\": true} to replace all data with the demo data");
        shop.seedDemo();
        return json({{"seeded", true}});
    }));

    // ---- CSV export (products | sales | profit | reorder)
    svr.Get("/api/export/:what", [&](const httplib::Request& req, httplib::Response& res) {
        try {
            std::string w = req.path_params.at("what");
            json rows;
            if (w == "products") rows = shop.listProducts("");
            else if (w == "sales") rows = shop.listSales("", 1000000);
            else if (w == "profit") rows = shop.profitView()["products"];
            else if (w == "reorder") rows = shop.reorderView(30)["items"];
            else throw ApiError(404, "Nothing to export called " + w);
            res.set_header("Content-Disposition", "attachment; filename=\"smartstock-" + w + ".csv\"");
            res.set_content(toCsv(rows), "text/csv");
        } catch (const ApiError& e) {
            res.status = e.status;
            res.set_content(json({{"error", e.what()}}).dump(), "application/json");
        }
    });

    // JSON errors for unknown API routes; static UI for everything else.
    svr.set_error_handler([](const httplib::Request& req, httplib::Response& res) {
        if (req.path.rfind("/api/", 0) == 0 && res.body.empty()) {  // keep messages set by handlers
            std::string msg = res.status == 404 ? "No such API endpoint" : "Request failed";
            res.set_content(json({{"error", msg}}).dump(), "application/json");
        }
    });
    svr.set_exception_handler([](const httplib::Request&, httplib::Response& res, std::exception_ptr) {
        res.status = 500;
        res.set_content(R"({"error":"Unexpected server error"})", "application/json");
    });
    svr.set_logger([](const httplib::Request& req, const httplib::Response& res) {
        if (req.method != "GET" || res.status >= 400)
            std::cout << req.method << " " << req.path << " -> " << res.status << "\n";
    });

    if (!svr.set_mount_point("/", webDir))
        std::cerr << "Warning: frontend folder '" << webDir << "' not found - API only.\n";

    std::cout << "SmartStock running at http://localhost:" << port << "  (data: " << dataDir << ", ui: " << webDir << ")\n"
              << "Press Ctrl+C to stop. Data is saved after every change.\n";
    if (!svr.listen("0.0.0.0", port)) {
        std::cerr << "Could not start server on port " << port << " (is it already in use?)\n";
        return 1;
    }
    return 0;
}
