

#include "Inventory.h"

Inventory::Inventory() {
}

int Inventory::getProductCount() const {
    return (int)products.size();
}

void Inventory::clear() {
    products.clear();
    skuIndexMap.clear();
}

Product* Inventory::getProductByIndex(int index) {
    if (index >= 0 && index < (int)products.size()) {
        return &products[index];
    }
    return nullptr;
}

const Product* Inventory::getProductByIndex(int index) const {
    if (index >= 0 && index < (int)products.size()) {
        return &products[index];
    }
    return nullptr;
}

std::vector<Product> Inventory::getAllProducts() const {
    return products;
}

bool Inventory::addProduct(const Product& product) {
    if (product.getSku().empty()) {
        return false;
    }
    
    if (skuIndexMap.find(product.getSku()) != skuIndexMap.end()) {
        return false;
    }
    products.push_back(product);
    skuIndexMap[product.getSku()] = (int)products.size() - 1;
    return true;
}

Product* Inventory::findProduct(const std::string& sku) {
    auto search = skuIndexMap.find(sku);
    if (search != skuIndexMap.end()) {
        int index = search->second;
        return &products[index];
    }
    return nullptr;
}

const Product* Inventory::findProduct(const std::string& sku) const {
    auto search = skuIndexMap.find(sku);
    if (search != skuIndexMap.end()) {
        int index = search->second;
        return &products[index];
    }
    return nullptr;
}

bool Inventory::updateStock(const std::string& sku, int qtyDelta, int expiryDate) {
    Product* p = findProduct(sku);
    if (p == nullptr) {
        return false;
    }
    if (qtyDelta < 0) {
        return p->reduceStock(-qtyDelta);      // a sale needs no expiry date
    } else if (qtyDelta > 0) {
        if (expiryDate == 0) {
            return false;                       // adding stock requires a real expiry
        }
        return p->addBatch(qtyDelta, expiryDate);
    }
    return true;
}

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

std::vector<std::string> Inventory::checkLowStock(int threshold) const {
    std::vector<std::string> lowStockSkus;
    for (int i = 0; i < (int)products.size(); i++) {
        if (products[i].getTotalStock() <= threshold) {
            lowStockSkus.push_back(products[i].getSku());
        }
    }
    return lowStockSkus;
}

std::vector<std::string> Inventory::checkOverstock(int threshold) const {
    std::vector<std::string> overstockedSkus;
    for (int i = 0; i < (int)products.size(); i++) {
        if (products[i].getTotalStock() >= threshold) {
            overstockedSkus.push_back(products[i].getSku());
        }
    }
    return overstockedSkus;
}

void Inventory::rebuildIndexMap() {
    skuIndexMap.clear();
    for (int i = 0; i < (int)products.size(); i++) {
        skuIndexMap[products[i].getSku()] = i;
    }
}
