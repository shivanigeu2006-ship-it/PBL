#ifndef INVENTORY_H
#define INVENTORY_H
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include <unordered_map>
#include "Product.h"

class Inventory {
private:
    std::vector<Product> products;
    std::unordered_map<std::string, int> skuIndexMap; 
public:
    Inventory();
    bool addProduct(const Product& product);
    Product* findProduct(const std::string& sku);
    const Product* findProduct(const std::string& sku) const;
    bool updateStock(const std::string& sku, int qtyDelta);
    void displayAll() const;
    std::vector<std::string> checkLowStock(int threshold) const;
    std::vector<std::string> checkOverstock(int threshold) const;
    std::vector<Product> getAllProducts() const;
    int getProductCount() const;
    Product* getProductByIndex(int index);
    const Product* getProductByIndex(int index) const;
    void rebuildIndexMap();
    void clear();
};
#endif
