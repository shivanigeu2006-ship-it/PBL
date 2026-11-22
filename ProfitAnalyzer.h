#ifndef PROFITANALYZER_H
#define PROFITANALYZER_H
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "Inventory.h"
#include "SalesRecord.h"

struct ProfitItem {
    std::string sku;
    std::string productName;
    double costPrice;
    double sellingPrice;
    int unitsSold;
    double profitPerUnit;
    double totalProfit;
    double profitMarginPercent;
};

struct ProfitSummary {
    std::vector<ProfitItem> items;
    double grandTotalProfit;
    double averageMargin;
};

class ProfitAnalyzer {
public:
    ProfitAnalyzer();
    double calculateProfitPerUnit(double sellingPrice, double costPrice) const;
    double calculateTotalProfit(double profitPerUnit, int unitsSold) const;
    double calculateProfitMargin(double sellingPrice, double costPrice) const;
    ProfitSummary analyzeProfits(const Inventory& inventory, const SalesRecord& salesRecord) const;
};
#endif
