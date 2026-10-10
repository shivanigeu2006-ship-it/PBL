// ============================================================
// Inventory.cpp  -  Module 3: Inventory
// Central place that stores all products.
// An unordered_map with the SKU as key gives fast lookup
// (average O(1) time).
// ============================================================
#ifndef INVENTORY_CPP
#define INVENTORY_CPP

#include <iostream>
#include <string>
#include <vector>
#include <unordered_map>
#include <algorithm>
#include "Product.cpp"
using namespace std;

class Inventory {
private:
    unordered_map<string, Product> products;   // key = SKU

public:
    // Add a new product. SKU must be non-empty and unique.
    bool addProduct(const Product& product) {
        if (product.getSku().empty()) {
            cout << "Error: SKU cannot be empty.\n";
            return false;
        }
        if (productExists(product.getSku())) {
            cout << "Error: SKU " << product.getSku() << " already exists.\n";
            return false;
        }
        products.insert(make_pair(product.getSku(), product));
        return true;
    }

    bool productExists(const string& sku) const {
        return products.find(sku) != products.end();
    }

    // Returns a pointer to the product, or nullptr if the SKU is not found
    Product* findProduct(const string& sku) {
        unordered_map<string, Product>::iterator it = products.find(sku);
        if (it == products.end()) return nullptr;
        return &(it->second);
    }

    const Product* findProduct(const string& sku) const {
        unordered_map<string, Product>::const_iterator it = products.find(sku);
        if (it == products.end()) return nullptr;
        return &(it->second);
    }

    // Add a new batch of stock to an existing product
    bool addStock(const string& sku, int quantity, const Date& expiry) {
        Product* p = findProduct(sku);
        if (p == nullptr) {
            cout << "Error: SKU " << sku << " not found.\n";
            return false;
        }
        return p->addBatch(quantity, expiry);
    }

    // Reduce stock (used by SalesRecord). FEFO is applied inside Product.
    bool sellStock(const string& sku, int units, const Date& today) {
        Product* p = findProduct(sku);
        if (p == nullptr) {
            cout << "Error: SKU " << sku << " not found.\n";
            return false;
        }
        return p->removeStockFEFO(units, today);
    }

    // Total stock of a product, or -1 if the SKU is not found
    int getStock(const string& sku) const {
        const Product* p = findProduct(sku);
        if (p == nullptr) return -1;
        return p->getTotalStock();
    }

    int getProductCount() const {
        return (int)products.size();
    }

    // All SKUs in sorted order (so output is always in the same order)
    vector<string> getAllSKUs() const {
        vector<string> skus;
        unordered_map<string, Product>::const_iterator it;
        for (it = products.begin(); it != products.end(); ++it) {
            skus.push_back(it->first);
        }
        sort(skus.begin(), skus.end());
        return skus;
    }

    void displayAll() const {
        vector<string> skus = getAllSKUs();
        if (skus.empty()) {
            cout << "Inventory is empty.\n";
            return;
        }
        for (size_t i = 0; i < skus.size(); i++) {
            findProduct(skus[i])->display();
        }
    }
};

#endif