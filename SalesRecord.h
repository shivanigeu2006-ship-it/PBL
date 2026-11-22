#ifndef SALESRECORD_H
#define SALESRECORD_H
#include <iostream>
#include <string>
#include <vector>
#include "Inventory.h"
#include "Shelf.h"
// Struct representing a single sale transaction
struct SaleEntry {
    std::string sku;
    int unitsSold;
    int dayNumber;
};
// SalesRecord class maintains sales history and coordinates stock/shelf updates
class SalesRecord {
private:
    std::vector<SaleEntry> sales;
public:
    SalesRecord();
    bool recordSale(const std::string& sku, int unitsSold, int dayNumber,
                    Inventory& inventory, std::vector<Shelf>& shelves);
    int getTotalUnitsSold(const std::string& sku) const;
    std::vector<SaleEntry> getAllSales() const;
    int getSaleCount() const;
    void addSaleDirect(const SaleEntry& entry);
    void clear();
};
#endif
