// ProfitAnalyzer.h - profit per unit, total profit and profit margin.
//   Profit per Unit = Selling Price - Cost Price
//   Total Profit    = Profit per Unit x Units Sold   (summed per sale so past prices stay correct)
//   Profit Margin   = (Selling Price - Cost Price) / Selling Price x 100
// Value of written-off (expired) stock is tracked as wastage and subtracted for net profit.
#pragma once
#include <vector>
#include <algorithm>
#include "SalesRecord.h"

struct Waste {
    std::string sku, name, batchId, date;
    int qty = 0;
    double cost = 0;  // cost price x qty
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(Waste, sku, name, batchId, date, qty, cost)

struct ProfitInfo {
    std::string sku, name;
    double costPrice = 0, sellPrice = 0, profitPerUnit = 0, marginPct = 0;
    int unitsSold = 0;
    double revenue = 0, totalProfit = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProfitInfo, sku, name, costPrice, sellPrice, profitPerUnit, marginPct,
                                   unitsSold, revenue, totalProfit)

struct ProfitSummary {
    double revenue = 0, costOfGoods = 0, grossProfit = 0, wastageLoss = 0, netProfit = 0, marginPct = 0;
    int unitsSold = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ProfitSummary, revenue, costOfGoods, grossProfit, wastageLoss, netProfit,
                                   marginPct, unitsSold)

class ProfitAnalyzer {
public:
    std::vector<ProfitInfo> perProduct(const Inventory& inv, const SalesRecord& sales) const {
        std::vector<ProfitInfo> out;
        for (auto* p : inv.all()) {
            ProfitInfo r;
            r.sku = p->sku(); r.name = p->name();
            r.costPrice = p->cost(); r.sellPrice = p->price();
            r.profitPerUnit = util::r2(p->profitPerUnit());
            r.marginPct = util::r2(p->marginPct());
            double rev = 0, prof = 0;
            for (auto& s : sales.all())
                if (s.sku == p->sku()) { r.unitsSold += s.units; rev += s.revenue(); prof += s.profit(); }
            r.revenue = util::r2(rev);
            r.totalProfit = util::r2(prof);
            out.push_back(r);
        }
        std::sort(out.begin(), out.end(), [](const ProfitInfo& a, const ProfitInfo& b) {
            return a.totalProfit > b.totalProfit;
        });
        return out;
    }

    ProfitSummary summary(const SalesRecord& sales, const std::vector<Waste>& waste) const {
        ProfitSummary s;
        for (auto& x : sales.all()) {
            s.revenue += x.revenue();
            s.costOfGoods += x.unitCost * x.units;
            s.unitsSold += x.units;
        }
        for (auto& w : waste) s.wastageLoss += w.cost;
        s.grossProfit = s.revenue - s.costOfGoods;
        s.netProfit = s.grossProfit - s.wastageLoss;
        s.marginPct = s.revenue > 0 ? s.grossProfit / s.revenue * 100.0 : 0.0;
        s.revenue = util::r2(s.revenue); s.costOfGoods = util::r2(s.costOfGoods);
        s.grossProfit = util::r2(s.grossProfit); s.wastageLoss = util::r2(s.wastageLoss);
        s.netProfit = util::r2(s.netProfit); s.marginPct = util::r2(s.marginPct);
        return s;
    }
};
