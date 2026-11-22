#ifndef REORDERMANAGER_H
#define REORDERMANAGER_H
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "Inventory.h"
#include "Supplier.h"
#include "DemandAnalyzer.h"
#include "SalesRecord.h"

struct ReorderItem {
    std::string sku;
    std::string productName;
    int currentStock;
    double avgDailyDemand;
    int leadTime;
    int safetyStock;
    double reorderPoint;
    int targetStock;
    int suggestedQuantity;
    bool needsReorder;
    bool limitedByShelf; 
};

class ReorderManager {
private:
    int defaultTargetStock;
public:
    ReorderManager();
    ReorderManager(int defaultTarget);
    double calculateReorderPoint(double avgDailyDemand, int leadTime, int safetyStock) const;
    int calculateSuggestedOrder(int targetStock, int currentStock) const;
    std::vector<ReorderItem> checkAllReorders(const Inventory& inventory,
                                             const std::vector<Supplier>& suppliers,
                                             const DemandAnalyzer& demandAnalyzer,
                                             const SalesRecord& salesRecord,
                                             int days) const;
};
#endif
