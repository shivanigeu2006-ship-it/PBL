// ============================================================================
// Module: SalesRecord.cpp (Module 4/11)
// Concept: Sales tracking with std::vector, stock validation, and shelf deduction
// Data Structure: std::vector<SaleEntry>
// ============================================================================
#include "SalesRecord.h"
// Default constructor
SalesRecord::SalesRecord() {
}
// Returns the total number of sales recorded
int SalesRecord::getSaleCount() const {
    return (int)sales.size();
}
// Returns a copy of all sales entries
std::vector<SaleEntry> SalesRecord::getAllSales() const {
    return sales;
}
// Clears all sales entries
void SalesRecord::clear() {
    sales.clear();
}
// Adds a sale directly without stock deduction (used when loading saved data)
void SalesRecord::addSaleDirect(const SaleEntry& entry) {
    sales.push_back(entry);
}
// Validates stock, executes FEFO stock reduction, updates shelf space, and records sale
bool SalesRecord::recordSale(const std::string& sku, int unitsSold, int dayNumber,
                             Inventory& inventory, std::vector<Shelf>& shelves) {
    // Basic input validation
    if (unitsSold <= 0 || dayNumber < 0) {
        std::cout << "Invalid sales parameters!\n";
        return false;
    }
    // Locate product using SKU
    Product* product = inventory.findProduct(sku);
    if (product == nullptr) {
        std::cout << "Product with SKU " << sku << " not found!\n";
        return false;
    }
    // Verify sufficient stock is available
    if (product->getTotalStock() < unitsSold) {
        std::cout << "Not enough stock! Available: " << product->getTotalStock()
                  << ", Requested: " << unitsSold << "\n";
        return false;
    }
    // Deduct stock using FEFO order
    bool reduced = product->reduceStock(unitsSold);
    if (!reduced) {
        return false;
    }
    // Update shelf used space
    int shelfId = product->getShelfId();
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].getShelfId() == shelfId) {
            shelves[i].removeUsed(unitsSold * product->getSpacePerUnit());
            break;
        }
    }
    // Record verified transaction
    SaleEntry newEntry;
    newEntry.sku = sku;
    newEntry.unitsSold = unitsSold;
    newEntry.dayNumber = dayNumber;
    sales.push_back(newEntry);
    std::cout << "Sale recorded successfully for SKU " << sku << "!\n";
    return true;
}
// Calculates total units sold for a specific SKU across all days
int SalesRecord::getTotalUnitsSold(const std::string& sku) const {
    int total = 0;
    for (int i = 0; i < (int)sales.size(); i++) {
        if (sales[i].sku == sku) {
            total = total + sales[i].unitsSold;
        }
    }
    return total;
}
