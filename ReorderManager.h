// ReorderManager.h - decides when and how much to reorder.
//   Reorder Point  = Average Daily Demand x Lead Time + Safety Stock
//   Suggested Qty  = Target Stock - Current Stock
// "Current stock" counts only sellable (non-expired) units.
#pragma once
#include <map>
#include <vector>
#include <cmath>
#include "DemandAnalyzer.h"
#include "Supplier.h"
#include "ShelfManager.h"

struct ReorderInfo {
    std::string sku, name, status, supplierId, supplierName, shelfId;
    int current = 0, expiredUnits = 0, leadTime = 0, safety = 0, target = 0;
    int suggestedQty = 0, orderQty = 0, shelfFree = -1;
    double avgDaily = 0, reorderPoint = 0, estCost = 0;
    bool spaceWarning = false;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ReorderInfo, sku, name, status, supplierId, supplierName, shelfId, current,
                                   expiredUnits, leadTime, safety, target, suggestedQty, orderQty, shelfFree,
                                   avgDaily, reorderPoint, estCost, spaceWarning)

class ReorderManager {
public:
    static constexpr double OVERSTOCK_FACTOR = 1.5;  // overstock when stock > 1.5 x target

    // status: out_of_stock | reorder | overstock | ok
    std::vector<ReorderInfo> analyze(const Inventory& inv, const SalesRecord& sales,
                                     const std::map<std::string, Supplier>& suppliers,
                                     const ShelfManager& shelves, const DemandAnalyzer& demand,
                                     long today, int days) const {
        std::vector<ReorderInfo> out;
        for (auto* p : inv.all()) {
            ReorderInfo r;
            r.sku = p->sku(); r.name = p->name(); r.shelfId = p->shelfId(); r.supplierId = p->supplierId();
            r.current = p->sellableStock(today);
            r.expiredUnits = p->expiredStock(today);
            r.target = p->target();

            int lead = 1, supplierSafety = 0;
            auto it = suppliers.find(p->supplierId());
            if (it != suppliers.end()) {
                lead = it->second.leadTime();
                supplierSafety = it->second.safetyStock();
                r.supplierName = it->second.name();
            }
            r.leadTime = lead;
            r.safety = p->safety() >= 0 ? p->safety() : supplierSafety;

            double avg = demand.rawAvg(p->sku(), sales, today, days);
            r.avgDaily = util::r2(avg);
            r.reorderPoint = util::r2(avg * lead + r.safety);

            r.suggestedQty = std::max(0, r.target - r.current);
            if (r.current == 0) r.status = "out_of_stock";
            else if (r.current <= r.reorderPoint) r.status = "reorder";
            else if (r.target > 0 && r.current > r.target * OVERSTOCK_FACTOR) r.status = "overstock";
            else r.status = "ok";

            r.orderQty = (r.status == "out_of_stock" || r.status == "reorder") ? r.suggestedQty : 0;
            r.estCost = util::r2(r.orderQty * p->cost());
            if (!p->shelfId().empty() && shelves.get(p->shelfId())) {
                r.shelfFree = shelves.freeFor(*p, p->shelfId()) - p->totalStock();
                r.spaceWarning = r.orderQty > r.shelfFree;
            }
            out.push_back(r);
        }
        return out;
    }
};
