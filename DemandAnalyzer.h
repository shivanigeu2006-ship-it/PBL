// DemandAnalyzer.h - average daily demand per product and High/Medium/Low classification.
// Avg Daily Demand = Total Units Sold / Number of Days.
// Products are SORTED by demand; the top third is High, middle third Medium, the rest Low
// (products that sold nothing are always Low).
#pragma once
#include <vector>
#include <algorithm>
#include <cmath>
#include "SalesRecord.h"

struct DemandInfo {
    std::string sku, name, level;
    int totalSold = 0;
    int days = 0;
    double avgDaily = 0;
    int rank = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(DemandInfo, sku, name, level, totalSold, days, avgDaily, rank)

class DemandAnalyzer {
public:
    static const int DEFAULT_DAYS = 30;

    // `days` is the look-back window. When the shop has fewer days of history than the window,
    // the history length is used instead so new shops are not under-counted. Never divides by zero.
    std::vector<DemandInfo> analyze(const Inventory& inv, const SalesRecord& sales, long today, int days) const {
        if (days <= 0) days = DEFAULT_DAYS;
        long from = today - days + 1;
        long first = sales.firstDay(today);
        int effective = (int)std::max<long>(1, std::min<long>(days, today - first + 1));

        std::vector<DemandInfo> out;
        for (auto* p : inv.all()) {
            DemandInfo d;
            d.sku = p->sku();
            d.name = p->name();
            d.days = effective;
            d.totalSold = sales.unitsSold(p->sku(), from, today);
            d.avgDaily = util::r2((double)d.totalSold / effective);
            out.push_back(d);
        }
        std::sort(out.begin(), out.end(), [](const DemandInfo& a, const DemandInfo& b) {
            if (a.totalSold != b.totalSold) return a.totalSold > b.totalSold;
            return a.sku < b.sku;
        });
        int n = (int)out.size();
        int highCut = (int)std::ceil(n / 3.0), midCut = (int)std::ceil(2.0 * n / 3.0);
        for (int i = 0; i < n; ++i) {
            out[i].rank = i + 1;
            out[i].level = (out[i].totalSold == 0) ? "Low" : (i < highCut ? "High" : (i < midCut ? "Medium" : "Low"));
        }
        return out;
    }

    // Unrounded average used by the reorder formula (so rounding never skews the reorder point).
    double rawAvg(const std::string& sku, const SalesRecord& sales, long today, int days) const {
        if (days <= 0) days = DEFAULT_DAYS;
        long first = sales.firstDay(today);
        int effective = (int)std::max<long>(1, std::min<long>(days, today - first + 1));
        return (double)sales.unitsSold(sku, today - days + 1, today) / effective;
    }
};
