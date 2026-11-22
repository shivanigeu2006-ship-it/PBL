

#include "DemandAnalyzer.h"

DemandAnalyzer::DemandAnalyzer() {
    highThreshold = 3.0;
    mediumThreshold = 1.0;
}

DemandAnalyzer::DemandAnalyzer(double highThresh, double medThresh) {
    highThreshold = highThresh;
    mediumThreshold = medThresh;
}

double DemandAnalyzer::calculateAverageDailyDemand(int totalUnitsSold, int days) const {
    if (days <= 0 || totalUnitsSold <= 0) {
        return 0.0;
    }
    return (double)totalUnitsSold / (double)days;
}

std::string DemandAnalyzer::classifyDemand(double avgDailyDemand) const {
    if (avgDailyDemand >= highThreshold) {
        return "High";
    } else if (avgDailyDemand >= mediumThreshold) {
        return "Medium";
    } else {
        return "Low";
    }
}

void DemandAnalyzer::sortDemandDescending(std::vector<DemandItem>& items) const {
    for (int i = 0; i < (int)items.size() - 1; i++) {
        for (int j = 0; j < (int)items.size() - i - 1; j++) {
            if (items[j].averageDailyDemand < items[j + 1].averageDailyDemand) {
                DemandItem temp = items[j];
                items[j] = items[j + 1];
                items[j + 1] = temp;
            }
        }
    }
}

std::vector<DemandItem> DemandAnalyzer::analyzeAll(const Inventory& inventory,
                                                  const SalesRecord& salesRecord,
                                                  int days) const {
    std::vector<DemandItem> results;
    std::vector<Product> products = inventory.getAllProducts();
    for (int i = 0; i < (int)products.size(); i++) {
        std::string sku = products[i].getSku();
        int sold = salesRecord.getTotalUnitsSold(sku);
        double avgDemand = calculateAverageDailyDemand(sold, days);
        DemandItem item;
        item.sku = sku;
        item.productName = products[i].getName();
        item.totalUnitsSold = sold;
        item.averageDailyDemand = avgDemand;
        item.demandCategory = classifyDemand(avgDemand);
        results.push_back(item);
    }
    sortDemandDescending(results);
    return results;
}
