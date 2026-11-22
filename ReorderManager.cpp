

#include "ReorderManager.h"

ReorderManager::ReorderManager() {
    defaultTargetStock = 50;
}

ReorderManager::ReorderManager(int defaultTarget) {
    if (defaultTarget < 1) {
        defaultTargetStock = 50;
    } else {
        defaultTargetStock = defaultTarget;
    }
}

double ReorderManager::calculateReorderPoint(double avgDailyDemand, int leadTime, int safetyStock) const {
    if (leadTime < 0) {
        leadTime = 0;
    }
    if (safetyStock < 0) {
        safetyStock = 0;
    }
    return (avgDailyDemand * (double)leadTime) + (double)safetyStock;
}

int ReorderManager::calculateSuggestedOrder(int targetStock, int currentStock) const {
    int suggested = targetStock - currentStock;
    if (suggested < 0) {
        return 0;
    }
    return suggested;
}

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
