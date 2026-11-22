#ifndef EXPIRYMANAGER_H
#define EXPIRYMANAGER_H
#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "Product.h"
#include "Inventory.h"
// Struct for an individual batch expiry entry
struct ExpiryItem {
    std::string sku;
    std::string productName;
    int quantity;
    int expiryDate;
    int daysLeft;
    std::string status; // "Expired", "Expiring Soon", "Good"
};
// Custom comparator struct for priority_queue (Min-Heap: earliest expiry date on top)
struct CompareExpiry {
    bool operator()(const ExpiryItem& a, const ExpiryItem& b) const {
        return a.expiryDate > b.expiryDate;
    }
};
// ExpiryManager class detects expired and soon-to-expire batches using FEFO min-heap
class ExpiryManager {
private:
    int thresholdDays; // Configurable days to flag expiring soon
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
