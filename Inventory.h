
#pragma once
#include <unordered_map>
#include <vector>
#include <algorithm>
#include "Product.h"

class Inventory {
    std::unordered_map<std::string, Product> items_;

public:
    bool exists(const std::string& sku) const { return items_.count(sku) > 0; }

    bool add(const Product& p) {
        return items_.emplace(p.sku(), p).second;  
    }
    Product* find(const std::string& sku) {
        auto it = items_.find(sku);
        return it == items_.end() ? nullptr : &it->second;
    }
    const Product* find(const std::string& sku) const {
        auto it = items_.find(sku);
        return it == items_.end() ? nullptr : &it->second;
    }
    bool remove(const std::string& sku) { return items_.erase(sku) > 0; }
    size_t size() const { return items_.size(); }
    void clear() { items_.clear(); }

    // Products sorted by SKU (stable output for UI and files).
    std::vector<const Product*> all() const {
        std::vector<const Product*> v;
        for (auto& kv : items_) v.push_back(&kv.second);
        std::sort(v.begin(), v.end(), [](const Product* a, const Product* b) { return a->sku() < b->sku(); });
        return v;
    }
    std::vector<Product*> allMutable() {
        std::vector<Product*> v;
        for (auto& kv : items_) v.push_back(&kv.second);
        std::sort(v.begin(), v.end(), [](const Product* a, const Product* b) { return a->sku() < b->sku(); });
        return v;
    }

    int totalUnits() const { int s = 0; for (auto& kv : items_) s += kv.second.totalStock(); return s; }
};
