// ============================================================================
// Module: Inventory.cpp (Module 3/11)
// Concept: Dynamic std::vector storage + std::unordered_map for O(1) SKU lookup
// Data Structures: std::vector<Product> and std::unordered_map<std::string, int>
// ============================================================================
#include "Inventory.h"
// Default constructor
Inventory::Inventory() {
}
// Returns total number of products
int Inventory::getProductCount() const {
    return (int)products.size();
}
// Clears all products and the SKU index map
void Inventory::clear() {
    products.clear();
    skuIndexMap.clear();
}
// Gets product pointer by vector index
Product* Inventory::getProductByIndex(int index) {
    if (index >= 0 && index < (int)products.size()) {
        return &products[index];
    }
    return nullptr;
}
// Gets const product pointer by vector index
const Product* Inventory::getProductByIndex(int index) const {
    if (index >= 0 && index < (int)products.size()) {
        return &products[index];
    }
    return nullptr;
}
// Returns a copy of all products
std::vector<Product> Inventory::getAllProducts() const {
    return products;
}
// Adds a new product to inventory, checking for duplicate SKU
bool Inventory::addProduct(const Product& product) {
    if (product.getSku().empty()) {
        return false;
    }
    // Check if SKU already exists in map
    if (skuIndexMap.find(product.getSku()) != skuIndexMap.end()) {
        return false;
    }
    products.push_back(product);
    skuIndexMap[product.getSku()] = (int)products.size() - 1;
    return true;
}
// Finds a product pointer by SKU in O(1) time
Product* Inventory::findProduct(const std::string& sku) {
    auto search = skuIndexMap.find(sku);
    if (search != skuIndexMap.end()) {
        int index = search->second;
        return &products[index];
    }
    return nullptr;
}
// Finds a const product pointer by SKU in O(1) time
const Product* Inventory::findProduct(const std::string& sku) const {
    auto search = skuIndexMap.find(sku);
    if (search != skuIndexMap.end()) {
        int index = search->second;
        return &products[index];
    }
    return nullptr;
}
// Updates stock quantity for a product
bool Inventory::updateStock(const std::string& sku, int qtyDelta) {
    Product* p = findProduct(sku);
    if (p == nullptr) {
        return false;
    }
    if (qtyDelta < 0) {
        return p->reduceStock(-qtyDelta);
    } else if (qtyDelta > 0) {
        // Add new batch with default future expiry date
        return p->addBatch(qtyDelta, 20271231);
    }
    return true;
}
// Displays all products in a formatted console table
void Inventory::displayAll() const {
    std::cout << "\n======================== INVENTORY CATALOG ========================\n";
    std::cout << std::left << std::setw(10) << "SKU"
              << std::setw(24) << "Name"
              << std::setw(10) << "CP (Rs.)"
              << std::setw(10) << "SP (Rs.)"
              << std::setw(10) << "Shelf"
              << std::setw(10) << "Stock" << "\n";
    std::cout << "--------------------------------------------------------------------\n";
    for (int i = 0; i < (int)products.size(); i++) {
        std::cout << std::left << std::setw(10) << products[i].getSku()
                  << std::setw(24) << products[i].getName()
                  << std::setw(10) << std::fixed << std::setprecision(2) << products[i].getCostPrice()
                  << std::setw(10) << std::fixed << std::setprecision(2) << products[i].getSellingPrice()
                  << std::setw(10) << products[i].getShelfId()
                  << std::setw(10) << products[i].getTotalStock() << "\n";
    }
}
// Identifies SKUs where total stock is at or below threshold
std::vector<std::string> Inventory::checkLowStock(int threshold) const {
    std::vector<std::string> lowStockSkus;
    for (int i = 0; i < (int)products.size(); i++) {
        if (products[i].getTotalStock() <= threshold) {
            lowStockSkus.push_back(products[i].getSku());
        }
    }
    return lowStockSkus;
}
// Identifies SKUs where total stock is at or above threshold
std::vector<std::string> Inventory::checkOverstock(int threshold) const {
    std::vector<std::string> overstockedSkus;
    for (int i = 0; i < (int)products.size(); i++) {
        if (products[i].getTotalStock() >= threshold) {
            overstockedSkus.push_back(products[i].getSku());
        }
    }
    return overstockedSkus;
}
// Rebuilds the SKU to index map from scratch
void Inventory::rebuildIndexMap() {
    skuIndexMap.clear();
    for (int i = 0; i < (int)products.size(); i++) {
        skuIndexMap[products[i].getSku()] = i;
    }
}
