// ============================================================================
// Module: ProfitAnalyzer.cpp (Module 10/11)
// Concept: Financial performance analysis using formula calculations
// Formulas: Profit per Unit = Selling Price - Cost Price
//           Total Profit = Profit per Unit * Units Sold
//           Profit Margin % = ((Selling Price - Cost Price) / Selling Price) * 100
// ============================================================================
#include "ProfitAnalyzer.h"
// Default constructor
ProfitAnalyzer::ProfitAnalyzer() {
}
// Profit per Unit = Selling Price - Cost Price
double ProfitAnalyzer::calculateProfitPerUnit(double sellingPrice, double costPrice) const {
    return sellingPrice - costPrice;
}
// Total Profit = Profit per Unit * Units Sold
double ProfitAnalyzer::calculateTotalProfit(double profitPerUnit, int unitsSold) const {
    if (unitsSold <= 0) {
        return 0.0;
    }
    return profitPerUnit * (double)unitsSold;
}
// Profit Margin % = ((Selling Price - Cost Price) / Selling Price) * 100
double ProfitAnalyzer::calculateProfitMargin(double sellingPrice, double costPrice) const {
    if (sellingPrice <= 0.0) {
        return 0.0;
    }
    return ((sellingPrice - costPrice) / sellingPrice) * 100.0;
}
// Generates profit breakdown for all products in inventory
ProfitSummary ProfitAnalyzer::analyzeProfits(const Inventory& inventory, const SalesRecord& salesRecord) const {
    ProfitSummary summary;
    summary.grandTotalProfit = 0.0;
    summary.averageMargin = 0.0;
    std::vector<Product> products = inventory.getAllProducts();
    double totalMarginSum = 0.0;
    int countedProducts = 0;
    for (int i = 0; i < (int)products.size(); i++) {
        std::string sku = products[i].getSku();
        double cp = products[i].getCostPrice();
        double sp = products[i].getSellingPrice();
        int sold = salesRecord.getTotalUnitsSold(sku);
        double unitProfit = calculateProfitPerUnit(sp, cp);
        double totProfit = calculateTotalProfit(unitProfit, sold);
        double margin = calculateProfitMargin(sp, cp);
        ProfitItem item;
        item.sku = sku;
        item.productName = products[i].getName();
        item.costPrice = cp;
        item.sellingPrice = sp;
        item.unitsSold = sold;
        item.profitPerUnit = unitProfit;
        item.totalProfit = totProfit;
        item.profitMarginPercent = margin;
        summary.items.push_back(item);
        summary.grandTotalProfit = summary.grandTotalProfit + totProfit;
        totalMarginSum = totalMarginSum + margin;
        countedProducts++;
    }
    if (countedProducts > 0) {
        summary.averageMargin = totalMarginSum / (double)countedProducts;
    }
    return summary;
}
