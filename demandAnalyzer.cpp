// ============================================================
// DemandAnalyzer.cpp  -  Module 6: DemandAnalyzer
// Formula : Average Daily Demand = Total Units Sold / Number of Days
// Products are SORTED by average daily demand (highest first).
// Then the sorted list is split into three equal-sized groups:
//     top third = High, middle third = Medium, last third = Low
// ============================================================
#ifndef DEMANDANALYZER_CPP
#define DEMANDANALYZER_CPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
#include "Inventory.cpp"
#include "SalesRecord.cpp"
using namespace std;

struct DemandInfo {
    string sku;
    double avgDailyDemand;
    string category;   // "High", "Medium" or "Low"

    DemandInfo() : sku(""), avgDailyDemand(0), category("Low") {}
};

class DemandAnalyzer {
private:
    vector<DemandInfo> results;   // kept in sorted order (highest demand first)

    // Comparison function used by sort(): higher demand comes first
    static bool compareDemand(const DemandInfo& a, const DemandInfo& b) {
        if (a.avgDailyDemand != b.avgDailyDemand) {
            return a.avgDailyDemand > b.avgDailyDemand;
        }
        return a.sku < b.sku;   // same demand: order by SKU
    }

public:
    // Average Daily Demand = Total Units Sold / Number of Days
    double calculateAverageDailyDemand(int totalUnitsSold, int days) const {
        if (days <= 0) {
            cout << "Error: number of days must be greater than 0.\n";
            return 0;
        }
        if (totalUnitsSold < 0) {
            cout << "Error: total units sold cannot be negative.\n";
            return 0;
        }
        return (double)totalUnitsSold / days;
    }

    // Calculates demand for every product and classifies it.
    // 'days' is the number of days the sales records cover.
    bool analyze(const Inventory& inventory, const SalesRecord& salesRecord, int days) {
        results.clear();
        if (days <= 0) {
            cout << "Error: number of days must be greater than 0.\n";
            return false;
        }

        vector<string> skus = inventory.getAllSKUs();
        if (skus.empty()) {
            cout << "Error: no products in inventory to analyze.\n";
            return false;
        }

        for (size_t i = 0; i < skus.size(); i++) {
            DemandInfo info;
            info.sku = skus[i];
            info.avgDailyDemand =
                calculateAverageDailyDemand(salesRecord.getTotalUnitsSold(skus[i]), days);
            results.push_back(info);
        }

        // Sorting: highest average daily demand first
        sort(results.begin(), results.end(), compareDemand);

        // Classification using the sorted order
        int n = (int)results.size();
        int highEnd = (n + 2) / 3;           // ceil(n / 3)
        int mediumEnd = (2 * n + 2) / 3;     // ceil(2n / 3)
        for (int i = 0; i < n; i++) {
            if (i < highEnd) results[i].category = "High";
            else if (i < mediumEnd) results[i].category = "Medium";
            else results[i].category = "Low";
        }
        return true;
    }

    // Returns -1 if the SKU was not analyzed
    double getAverageDemand(const string& sku) const {
        for (size_t i = 0; i < results.size(); i++) {
            if (results[i].sku == sku) return results[i].avgDailyDemand;
        }
        return -1;
    }

    // Returns "Unknown" if the SKU was not analyzed
    string getCategory(const string& sku) const {
        for (size_t i = 0; i < results.size(); i++) {
            if (results[i].sku == sku) return results[i].category;
        }
        return "Unknown";
    }

    const vector<DemandInfo>& getResults() const { return results; }

    void displayReport() const {
        if (results.empty()) {
            cout << "No demand analysis available. Run analyze() first.\n";
            return;
        }
        cout << "Demand report (highest demand first):\n";
        for (size_t i = 0; i < results.size(); i++) {
            cout << "  " << results[i].sku << " | Avg daily demand: "
                 << results[i].avgDailyDemand << " | " << results[i].category << "\n";
        }
    }
};

#endif