

#include "ProfitAnalyzer.h"

ProfitAnalyzer::ProfitAnalyzer() {
}

double ProfitAnalyzer::calculateProfitPerUnit(double sellingPrice, double costPrice) const {
    return sellingPrice - costPrice;
}

double ProfitAnalyzer::calculateTotalProfit(double profitPerUnit, int unitsSold) const {
    if (unitsSold <= 0) {
        return 0.0;
    }
    return profitPerUnit * (double)unitsSold;
}

double ProfitAnalyzer::calculateProfitMargin(double sellingPrice, double costPrice) const {
    if (sellingPrice <= 0.0) {
        return 0.0;
    }
    return ((sellingPrice - costPrice) / sellingPrice) * 100.0;
}

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
