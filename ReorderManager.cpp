// ============================================================================
// Module: ReorderManager.cpp (Module 8/11)
// Concept: Formula-based reorder calculations for timely restocking
// Formulas: Reorder Point = (Avg Daily Demand * Lead Time) + Safety Stock
//           Suggested Order Quantity = Target Stock - Current Stock (minimum 0)
// ============================================================================
#include "ReorderManager.h"
// Default constructor
ReorderManager::ReorderManager() {
    defaultTargetStock = 50;
}
// Parameterized constructor
ReorderManager::ReorderManager(int defaultTarget) {
    if (defaultTarget < 1) {
        defaultTargetStock = 50;
    } else {
        defaultTargetStock = defaultTarget;
    }
}
// Reorder Point = (Average Daily Demand * Lead Time) + Safety Stock
double ReorderManager::calculateReorderPoint(double avgDailyDemand, int leadTime, int safetyStock) const {
    if (leadTime < 0) {
        leadTime = 0;
    }
    if (safetyStock < 0) {
        safetyStock = 0;
    }
    return (avgDailyDemand * (double)leadTime) + (double)safetyStock;
}
// Suggested Order Quantity = Target Stock - Current Stock (never negative)
int ReorderManager::calculateSuggestedOrder(int targetStock, int currentStock) const {
    int suggested = targetStock - currentStock;
    if (suggested < 0) {
        return 0;
    }
    return suggested;
}
// Computes reorder needs for all inventory items
std::vector<ReorderItem> ReorderManager::checkAllReorders(const Inventory& inventory,
                                                         const std::vector<Supplier>& suppliers,
                                                         const DemandAnalyzer& demandAnalyzer,
                                                         const SalesRecord& salesRecord,
                                                         int days) const {
    std::vector<ReorderItem> reports;
    std::vector<Product> products = inventory.getAllProducts();
    for (int i = 0; i < (int)products.size(); i++) {
        std::string sku = products[i].getSku();
        int currentStock = products[i].getTotalStock();
        // Find supplier parameters for this SKU or fallback to defaults
        int leadTime = 3;
        int safetyStock = 10;
        for (int s = 0; s < (int)suppliers.size(); s++) {
            if (suppliers[s].getSku() == sku) {
                leadTime = suppliers[s].getLeadTime();
                safetyStock = suppliers[s].getSafetyStock();
                break;
            }
        }
        int sold = salesRecord.getTotalUnitsSold(sku);
        double avgDemand = demandAnalyzer.calculateAverageDailyDemand(sold, days);
        double rop = calculateReorderPoint(avgDemand, leadTime, safetyStock);
        // Target stock is dynamically set to ROP + buffer or defaultTargetStock
        int target = (int)rop + 20;
        if (target < defaultTargetStock) {
            target = defaultTargetStock;
        }
        int suggestedQty = calculateSuggestedOrder(target, currentStock);
        bool reorderNeeded = (currentStock <= (int)rop);
        ReorderItem item;
        item.sku = sku;
        item.productName = products[i].getName();
        item.currentStock = currentStock;
        item.avgDailyDemand = avgDemand;
        item.leadTime = leadTime;
        item.safetyStock = safetyStock;
        item.reorderPoint = rop;
        item.targetStock = target;
        item.suggestedQuantity = (reorderNeeded ? suggestedQty : 0);
        item.needsReorder = reorderNeeded;
        item.limitedByShelf = false;
        reports.push_back(item);
    }
    return reports;
}
