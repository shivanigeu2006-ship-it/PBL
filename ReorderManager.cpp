// ============================================================
// ReorderManager.cpp  -  Module 8: ReorderManager
// Formulas (exactly as in the project report):
//   Reorder Point  = Average Daily Demand x Lead Time + Safety Stock
//   Suggested Order = Target Stock - Current Stock
// A reorder is needed when Current Stock <= Reorder Point.
// ============================================================
#ifndef REORDERMANAGER_CPP
#define REORDERMANAGER_CPP

#include <iostream>
#include <string>
#include "Product.cpp"
#include "Supplier.cpp"
using namespace std;

struct ReorderInfo {
    string sku;
    int currentStock;
    double reorderPoint;
    bool reorderNeeded;
    int suggestedOrder;

    ReorderInfo() : sku(""), currentStock(0), reorderPoint(0),
                    reorderNeeded(false), suggestedOrder(0) {}
};

class ReorderManager {
public:
    // Reorder Point = Average Daily Demand x Lead Time + Safety Stock
    double calculateReorderPoint(double avgDailyDemand, int leadTime, int safetyStock) const {
        if (avgDailyDemand < 0 || leadTime < 0 || safetyStock < 0) {
            cout << "Error: demand, lead time and safety stock cannot be negative.\n";
            return 0;
        }
        return avgDailyDemand * leadTime + safetyStock;
    }

    // Reorder when current stock has fallen to the reorder point or below
    bool isReorderNeeded(int currentStock, double reorderPoint) const {
        return currentStock <= reorderPoint;
    }

    // Suggested Order = Target Stock - Current Stock
    // (a negative result means we already have enough, so it is shown as 0)
    int calculateSuggestedOrder(int targetStock, int currentStock) const {
        if (targetStock < 0 || currentStock < 0) {
            cout << "Error: target stock and current stock cannot be negative.\n";
            return 0;
        }
        int order = targetStock - currentStock;
        if (order < 0) order = 0;
        return order;
    }

    // Full reorder analysis for one product
    ReorderInfo analyzeProduct(const Product& product, double avgDailyDemand,
                               const Supplier& supplier, int targetStock) const {
        ReorderInfo info;
        info.sku = product.getSku();
        info.currentStock = product.getTotalStock();
        info.reorderPoint = calculateReorderPoint(avgDailyDemand,
                                                  supplier.getLeadTimeDays(),
                                                  supplier.getSafetyStock());
        info.reorderNeeded = isReorderNeeded(info.currentStock, info.reorderPoint);
        info.suggestedOrder = calculateSuggestedOrder(targetStock, info.currentStock);
        return info;
    }

    void displayReorderInfo(const ReorderInfo& info) const {
        cout << "  " << info.sku << " | Current stock: " << info.currentStock
             << " | Reorder point: " << info.reorderPoint << " | ";
        if (info.reorderNeeded) {
            cout << "REORDER NEEDED, suggested order: " << info.suggestedOrder << " units\n";
        } else {
            cout << "No reorder needed\n";
        }
    }
};

#endif