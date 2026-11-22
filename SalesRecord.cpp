

#include "SalesRecord.h"

SalesRecord::SalesRecord() {
}

int SalesRecord::getSaleCount() const {
    return (int)sales.size();
}

std::vector<SaleEntry> SalesRecord::getAllSales() const {
    return sales;
}

void SalesRecord::clear() {
    sales.clear();
}

void SalesRecord::addSaleDirect(const SaleEntry& entry) {
    sales.push_back(entry);
}

bool SalesRecord::recordSale(const std::string& sku, int unitsSold, int dayNumber,
                             Inventory& inventory, std::vector<Shelf>& shelves) {
    
    if (unitsSold <= 0 || dayNumber < 0) {
        std::cout << "Invalid sales parameters!\n";
        return false;
    }
    
    Product* product = inventory.findProduct(sku);
    if (product == nullptr) {
        std::cout << "Product with SKU " << sku << " not found!\n";
        return false;
    }
    
    if (product->getTotalStock() < unitsSold) {
        std::cout << "Not enough stock! Available: " << product->getTotalStock()
                  << ", Requested: " << unitsSold << "\n";
        return false;
    }
    
    bool reduced = product->reduceStock(unitsSold);
    if (!reduced) {
        return false;
    }
    
    int shelfId = product->getShelfId();
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].getShelfId() == shelfId) {
            shelves[i].removeUsed(unitsSold * product->getSpacePerUnit());
            break;
        }
    }
    
    SaleEntry newEntry;
    newEntry.sku = sku;
    newEntry.unitsSold = unitsSold;
    newEntry.dayNumber = dayNumber;
    sales.push_back(newEntry);
    std::cout << "Sale recorded successfully for SKU " << sku << "!\n";
    return true;
}

int SalesRecord::getTotalUnitsSold(const std::string& sku) const {
    int total = 0;
    for (int i = 0; i < (int)sales.size(); i++) {
        if (sales[i].sku == sku) {
            total = total + sales[i].unitsSold;
        }
    }
    return total;
}
