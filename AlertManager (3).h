// AlertManager.h - turns the analysis results into plain-language smart alerts.
// Severity: critical (act today), warning (plan soon), info (good to know).
#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include "ShelfManager.h"
#include "SalesRecord.h"
#include "Supplier.h"
#include "DemandAnalyzer.h"
#include "ExpiryManager.h"
#include "ReorderManager.h"

struct Alert {
    std::string type, severity, sku, message;  // severity: critical | warning | info
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Alert, type, severity, sku, message)

class AlertManager {
    static std::string money(double v) { char b[32]; std::snprintf(b, sizeof b, "Rs.%.2f", v); return b; }
    static std::string whole(double v) { char b[32]; std::snprintf(b, sizeof b, "%.1f", v); return b; }

public:
    std::vector<Alert> build(const Inventory& inv_, const ShelfManager& shelves_, const SalesRecord& sales_,
                             const std::map<std::string, Supplier>& suppliers_, const DemandAnalyzer& demand_,
                             const ExpiryManager& expiry_, const ReorderManager& reorder_, long t, int WINDOW,
                             int SOON_DAYS) const {
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
};
