#ifndef DEMANDANALYZER_H
#define DEMANDANALYZER_H
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "Inventory.h"
#include "SalesRecord.h"
// Struct for demand analysis report per product
struct DemandItem {
    std::string sku;
    std::string productName;
    int totalUnitsSold;
    double averageDailyDemand;
    std::string demandCategory; // "High", "Medium", or "Low"
};
// DemandAnalyzer class calculates average daily demand and categorizes products
class DemandAnalyzer {
private:
    double highThreshold;
    double mediumThreshold;
public:
    DemandAnalyzer();
    DemandAnalyzer(double highThresh, double medThresh);
    double calculateAverageDailyDemand(int totalUnitsSold, int days) const;
    std::string classifyDemand(double avgDailyDemand) const;
    void sortDemandDescending(std::vector<DemandItem>& items) const;
    std::vector<DemandItem> analyzeAll(const Inventory& inventory,
                                       const SalesRecord& salesRecord,
                                       int days) const;
};
#endif
