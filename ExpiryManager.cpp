

#include "ExpiryManager.h"

ExpiryManager::ExpiryManager() {
    thresholdDays = 7;
}

ExpiryManager::ExpiryManager(int threshold) {
    if (threshold < 1) {
        thresholdDays = 7;
    } else {
        thresholdDays = threshold;
    }
}

int ExpiryManager::getThresholdDays() const {
    return thresholdDays;
}

void ExpiryManager::setThresholdDays(int days) {
    if (days >= 1) {
        thresholdDays = days;
    }
}

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

std::vector<ExpiryItem> ExpiryManager::getAllExpiryStatus(const Inventory& inventory, int currentDate) const {
    std::vector<ExpiryItem> allList;
    std::priority_queue<ExpiryItem, std::vector<ExpiryItem>, CompareExpiry> pq = buildExpiryQueue(inventory, currentDate);
    while (!pq.empty()) {
        allList.push_back(pq.top());
        pq.pop();
    }
    return allList;
}

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
