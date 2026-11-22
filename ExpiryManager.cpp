// ============================================================================
// Module: ExpiryManager.cpp (Module 7/11)
// Concept: Priority Queue (Min-Heap) for FEFO Expiry Tracking
// Data Structure: std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry>
// ============================================================================
#include "ExpiryManager.h"
// Default constructor sets 7-day alert threshold
ExpiryManager::ExpiryManager() {
    thresholdDays = 7;
}
// Parameterized constructor sets custom threshold
ExpiryManager::ExpiryManager(int threshold) {
    if (threshold < 1) {
        thresholdDays = 7;
    } else {
        thresholdDays = threshold;
    }
}
// Returns the threshold in days
int ExpiryManager::getThresholdDays() const {
    return thresholdDays;
}
// Sets the threshold in days
void ExpiryManager::setThresholdDays(int days) {
    if (days >= 1) {
        thresholdDays = days;
    }
}
// Builds a priority queue of all batches, placing nearest expiry on top
std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry>
ExpiryManager::buildExpiryQueue(const Inventory& inventory, int currentDate) const {
    std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry> pq;
    std::vector<Product> products = inventory.getAllProducts();
    for (int i = 0; i < (int)products.size(); i++) {
        std::vector<Batch> batches = products[i].getBatches();
        for (int j = 0; j < (int)batches.size(); j++) {
            if (batches[j].quantity > 0) {
                int daysLeft = daysDifference(currentDate, batches[j].expiryDate);
                ExpiryItem item;
                item.sku = products[i].getSku();
                item.productName = products[i].getName();
                item.quantity = batches[j].quantity;
                item.expiryDate = batches[j].expiryDate;
                item.daysLeft = daysLeft;
                if (daysLeft < 0) {
                    item.status = "Expired";
                } else if (daysLeft <= thresholdDays) {
                    item.status = "Expiring Soon";
                } else {
                    item.status = "Good";
                }
                pq.push(item);
            }
        }
    }
    return pq;
}
// Returns list of batches that have passed their expiry date
std::vector<ExpiryItem> ExpiryManager::listExpired(const Inventory& inventory, int currentDate) const {
    std::vector<ExpiryItem> expiredList;
    std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry> pq = buildExpiryQueue(inventory, currentDate);
    while (!pq.empty()) {
        ExpiryItem item = pq.top();
        pq.pop();
        if (item.daysLeft < 0) {
            expiredList.push_back(item);
        }
    }
    return expiredList;
}
// Returns list of batches expiring soon within thresholdDays
std::vector<ExpiryItem> ExpiryManager::listExpiringSoon(const Inventory& inventory, int currentDate) const {
    std::vector<ExpiryItem> soonList;
    std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry> pq = buildExpiryQueue(inventory, currentDate);
    while (!pq.empty()) {
        ExpiryItem item = pq.top();
        pq.pop();
        if (item.daysLeft >= 0 && item.daysLeft <= thresholdDays) {
            soonList.push_back(item);
        }
    }
    return soonList;
}
// Returns all batches ordered by earliest expiry first (FEFO order)
std::vector<ExpiryItem> ExpiryManager::getAllExpiryStatus(const Inventory& inventory, int currentDate) const {
    std::vector<ExpiryItem> allList;
    std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry> pq = buildExpiryQueue(inventory, currentDate);
    while (!pq.empty()) {
        allList.push_back(pq.top());
        pq.pop();
    }
    return allList;
}
// Finds the next FEFO batch to be dispatched for a given SKU
bool ExpiryManager::getFEFOBatch(const std::string& sku, const Inventory& inventory, ExpiryItem& nextBatch) const {
    const Product* product = inventory.findProduct(sku);
    if (product == nullptr) {
        return false;
    }
    std::vector<Batch> batches = product->getBatches();
    if (batches.empty()) {
        return false;
    }
    nextBatch.sku = product->getSku();
    nextBatch.productName = product->getName();
    nextBatch.quantity = batches[0].quantity;
    nextBatch.expiryDate = batches[0].expiryDate;
    nextBatch.daysLeft = 0;
    nextBatch.status = "FEFO Top";
    return true;
}
