// test_core.cpp - unit tests for the core modules (no web server needed).
// Build & run:  g++ -std=c++17 -Iinclude -Isrc tests/test_core.cpp -o test_core && ./test_core
#include <iostream>
#include "SmartShop.h"

static int passed = 0, failed = 0;
#define CHECK(cond)                                                                   \
    do {                                                                              \
        if (cond) { ++passed; }                                                       \
        else { ++failed; std::cout << "  FAIL line " << __LINE__ << ": " #cond "\n"; } \
    } while (0)

int main() {
    const long T = util::daysFromCivil(2026, 6, 15);  // fixed "today" for repeatable tests

    std::cout << "Dates\n";
    long d;
    CHECK(util::parseDate("2026-06-15", d) && d == T);
    CHECK(util::formatDate(T) == "2026-06-15");
    CHECK(!util::parseDate("2026-02-30", d));
    CHECK(!util::parseDate("junk", d));
    CHECK(util::parseDate("", d) && d == util::NO_EXPIRY);

    std::cout << "Product & FEFO\n";
    Product p("A", "Apple", "Grocery", 10, 15, 50);
    p.addBatch(10, T + 9, T);
    p.addBatch(5, T + 2, T);
    p.addBatch(4, T - 1, T);  // already expired
    CHECK(p.totalStock() == 19 && p.sellableStock(T) == 15 && p.expiredStock(T) == 4);
    CHECK(p.sell(7, T) == 7);                       // 5 from the T+2 batch, 2 from T+9
    CHECK(p.batches().size() == 2);                 // empty batch removed, expired one untouched
    CHECK(p.sellableStock(T) == 8);
    CHECK(p.sell(100, T) == 8);                     // never sells expired stock
    CHECK(p.expiredStock(T) == 4);
    CHECK(p.discardExpired(T).size() == 1 && p.totalStock() == 0);
    CHECK(p.profitPerUnit() == 5 && std::abs(p.marginPct() - 33.3333) < 0.01);

    std::cout << "Inventory\n";
    Inventory inv;
    CHECK(inv.add(Product("X", "x", "c", 1, 2, 3)));
    CHECK(!inv.add(Product("X", "dup", "c", 1, 2, 3)));
    CHECK(inv.find("X") && !inv.find("Y"));

    std::cout << "Shelves\n";
    ShelfManager sm;
    sm.add(Shelf("S1", "Shelf 1", "Grocery", 20));
    sm.add(Shelf("S2", "Shelf 2", "General", 100));
    Product q("Q", "Quinoa", "Grocery", 1, 2, 10);
    q.addBatch(15, util::NO_EXPIRY, T);
    inv.add(q);
    std::string err;
    CHECK(sm.assign(inv, "Q", "S1", err));
    CHECK(sm.get("S1")->used() == 15 && sm.get("S1")->freeSpace() == 5);
    inv.add(Product("R", "Rye", "Grocery", 1, 2, 10));
    inv.find("R")->addBatch(10, util::NO_EXPIRY, T);
    CHECK(!sm.assign(inv, "R", "S1", err));         // 10 does not fit in the 5 left
    CHECK(sm.assign(inv, "R", "S2", err));
    CHECK(sm.autoAssign(*inv.find("R"), 3) == "S2" || sm.autoAssign(*inv.find("R"), 3) == "S1");

    std::cout << "Sales\n";
    SalesRecord sales;
    auto r = sales.record(inv, "Q", 5, T);
    CHECK(r.ok && inv.find("Q")->totalStock() == 10);
    CHECK(!sales.record(inv, "Q", 11, T).ok);       // more than available
    CHECK(!sales.record(inv, "ZZZ", 1, T).ok);
    CHECK(!sales.record(inv, "Q", 0, T).ok);
    CHECK(r.sale.revenue() == 10 && r.sale.profit() == 5);

    std::cout << "Demand (Avg Daily Demand = units sold / days)\n";
    SalesRecord hist;
    inv.find("Q")->addBatch(1000, util::NO_EXPIRY, T);
    for (int i = 0; i < 10; ++i) hist.record(inv, "Q", 3, T - i);   // 30 units over 10 days
    DemandAnalyzer da;
    auto dm = da.analyze(inv, hist, T, 10);
    double qAvg = 0;
    for (auto& x : dm) if (x.sku == "Q") qAvg = x.avgDaily;
    CHECK(std::abs(qAvg - 3.0) < 1e-9);
    CHECK(da.analyze(inv, hist, T, 0).size() == inv.size());          // days = 0 is safe
    CHECK(dm[0].sku == "Q" && dm[0].level == "High");
    CHECK(dm.back().level == "Low");

    std::cout << "Reorder (ROP = avg x lead + safety; qty = target - current)\n";
    std::map<std::string, Supplier> sup;
    sup["SUP"] = Supplier("SUP", "Sup", "", 4, 6);
    inv.find("Q")->setSupplier("SUP");
    inv.find("Q")->setTarget(100);
    SalesRecord none;
    SalesRecord s3;
    for (int i = 0; i < 10; ++i) s3.record(inv, "Q", 3, T - i);
    // stock is now 1000 + 10 - 30 (hist) - 30 (s3) = 950 -> force a low value to test a reorder
    Product& qq = *inv.find("Q");
    qq.sell(qq.totalStock() - 20, T);                 // leave 20 units
    ReorderManager rm;
    auto ro = rm.analyze(inv, s3, sup, sm, da, T, 10);
    for (auto& x : ro) if (x.sku == "Q") {
        CHECK(std::abs(x.reorderPoint - (3.0 * 4 + 6)) < 1e-9);    // 18
        CHECK(x.current == 20 && x.status == "ok");                // 20 > 18
        CHECK(x.suggestedQty == 80);
    }
    qq.sell(5, T);                                    // 15 <= 18 -> reorder
    ro = rm.analyze(inv, s3, sup, sm, da, T, 10);
    for (auto& x : ro) if (x.sku == "Q") CHECK(x.status == "reorder" && x.orderQty == 85);

    std::cout << "Expiry manager (priority queue)\n";
    Inventory e;
    Product a("A", "a", "c", 2, 3, 5), b("B", "b", "c", 2, 3, 5);
    a.addBatch(1, T + 5, T); a.addBatch(1, T - 3, T);
    b.addBatch(1, T + 1, T); b.addBatch(1, T + 30, T);
    e.add(a); e.add(b);
    ExpiryManager em;
    auto q2 = em.fefoQueue(e, T, 7);
    CHECK(q2.size() == 4);
    CHECK(q2[0].status == "expired" && q2[1].status == "critical" && q2[2].status == "soon" && q2[3].status == "ok");
    CHECK(em.expired(e, T).size() == 1 && em.expiringSoon(e, T, 7).size() == 2);

    std::cout << "Profit\n";
    ProfitAnalyzer pa;
    auto summary = pa.summary(hist, {});
    CHECK(summary.unitsSold == 30 && std::abs(summary.revenue - 60) < 1e-9);   // price 2 x 30 units
    CHECK(std::abs(summary.grossProfit - 30) < 1e-9 && std::abs(summary.marginPct - 50) < 1e-9);

    std::cout << "SmartShop persistence\n";
    {
        std::string dir = "/tmp/ss_unit";
        std::filesystem::remove_all(dir);
        SmartShop shop(dir);                         // seeds demo data and saves it
        int n1 = (int)shop.listProducts("").size();
        shop.addProduct({{"sku", "NEW-1"}, {"name", "New"}, {"cost", 5}, {"price", 9}, {"target", 10}});
        SmartShop reopened(dir);                     // loads from files
        CHECK((int)reopened.listProducts("").size() == n1 + 1);
        CHECK(reopened.getProduct("NEW-1")["name"] == "New");
    }

    std::cout << "\n" << passed << " passed, " << failed << " failed\n";
    return failed ? 1 : 0;
}
