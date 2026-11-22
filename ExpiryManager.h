#ifndef EXPIRYMANAGER_H
#define EXPIRYMANAGER_H
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "Product.h"
#include "Inventory.h"

struct ExpiryItem {
    std::string sku;
    std::string productName;
    int quantity;
    int expiryDate;
    int daysLeft;
    std::string status; 
};

struct CompareExpiry {
    bool operator()(const ExpiryItem& a, const ExpiryItem& b) const {
        return a.expiryDate > b.expiryDate;
    }
};

class ExpiryManager {
private:
    int thresholdDays; 
public:
    ExpiryManager();
    ExpiryManager(int threshold);
    std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry>
    buildExpiryQueue(const Inventory& inventory, int currentDate) const;
    std::vector<ExpiryItem> listExpired(const Inventory& inventory, int currentDate) const;
    std::vector<ExpiryItem> listExpiringSoon(const Inventory& inventory, int currentDate) const;
    std::vector<ExpiryItem> getAllExpiryStatus(const Inventory& inventory, int currentDate) const;
    bool getFEFOBatch(const std::string& sku, const Inventory& inventory, ExpiryItem& nextBatch) const;
    int getThresholdDays() const;
    void setThresholdDays(int days);
};
#endif
