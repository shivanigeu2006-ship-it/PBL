// ============================================================
// ExpiryManager.cpp  -  Module 7: ExpiryManager
// Uses a priority_queue (min-heap on expiry date) so the batch with
// the NEAREST expiry date is always at the top. This is FEFO:
// First Expiry First Out.
//
// Rules used:
//   Expired       : expiry date is before today
//   Expiring soon : 0 to 'soonDays' days left (default 7 days)
// ============================================================
#ifndef EXPIRYMANAGER_CPP
#define EXPIRYMANAGER_CPP

#include <iostream>
#include <string>
#include <vector>
#include <queue>
#include "Inventory.cpp"
using namespace std;

struct ExpiryEntry {
    string sku;
    int quantity;
    Date expiry;

    ExpiryEntry() : sku(""), quantity(0) {}
    ExpiryEntry(const string& s, int q, const Date& e) : sku(s), quantity(q), expiry(e) {}
};

// Decides the priority order. priority_queue puts the "largest" item on
// top, so we say "a is lower priority than b" when a expires LATER than b.
struct CompareExpiry {
    bool operator()(const ExpiryEntry& a, const ExpiryEntry& b) const {
        int dayA = a.expiry.toDayNumber();
        int dayB = b.expiry.toDayNumber();
        if (dayA != dayB) return dayA > dayB;
        return a.sku > b.sku;
    }
};

class ExpiryManager {
private:
    priority_queue<ExpiryEntry, vector<ExpiryEntry>, CompareExpiry> fefoQueue;
    int soonDays;   // "expiring soon" limit in days

public:
    ExpiryManager() : soonDays(7) {}

    explicit ExpiryManager(int days) : soonDays(7) {
        setSoonDays(days);
    }

    bool setSoonDays(int days) {
        if (days < 0) {
            cout << "Error: days cannot be negative.\n";
            return false;
        }
        soonDays = days;
        return true;
    }

    int getSoonDays() const { return soonDays; }

    // Puts every batch with stock into the priority queue
    void loadBatches(const Inventory& inventory) {
        fefoQueue = priority_queue<ExpiryEntry, vector<ExpiryEntry>, CompareExpiry>();

        vector<string> skus = inventory.getAllSKUs();
        for (size_t i = 0; i < skus.size(); i++) {
            const Product* p = inventory.findProduct(skus[i]);
            const vector<Batch>& batches = p->getBatches();
            for (size_t j = 0; j < batches.size(); j++) {
                if (batches[j].quantity > 0) {
                    fefoQueue.push(ExpiryEntry(skus[i], batches[j].quantity, batches[j].expiry));
                }
            }
        }
    }

    int getQueueSize() const { return (int)fefoQueue.size(); }

    // The batch that expires first (it may already be expired)
    bool getEarliestBatch(ExpiryEntry& result) const {
        if (fefoQueue.empty()) return false;
        result = fefoQueue.top();
        return true;
    }

    // FEFO choice: the earliest-expiring batch that is NOT expired yet
    bool getNextBatchToSell(const Date& today, ExpiryEntry& result) const {
        priority_queue<ExpiryEntry, vector<ExpiryEntry>, CompareExpiry> temp = fefoQueue;
        while (!temp.empty()) {
            ExpiryEntry e = temp.top();
            temp.pop();
            if (!e.expiry.isBefore(today)) {
                result = e;
                return true;
            }
        }
        return false;
    }

    // All expired batches (earliest expiry first)
    vector<ExpiryEntry> getExpiredBatches(const Date& today) const {
        vector<ExpiryEntry> list;
        priority_queue<ExpiryEntry, vector<ExpiryEntry>, CompareExpiry> temp = fefoQueue;
        while (!temp.empty()) {
            ExpiryEntry e = temp.top();
            temp.pop();
            if (e.expiry.isBefore(today)) list.push_back(e);
        }
        return list;
    }

    // All batches that will expire within 'soonDays' days (not yet expired)
    vector<ExpiryEntry> getExpiringSoonBatches(const Date& today) const {
        vector<ExpiryEntry> list;
        priority_queue<ExpiryEntry, vector<ExpiryEntry>, CompareExpiry> temp = fefoQueue;
        while (!temp.empty()) {
            ExpiryEntry e = temp.top();
            temp.pop();
            int daysLeft = today.daysUntil(e.expiry);
            if (daysLeft >= 0 && daysLeft <= soonDays) list.push_back(e);
        }
        return list;
    }

    // Shows all batches in FEFO order with their status
    void displayFEFOOrder(const Date& today) const {
        if (fefoQueue.empty()) {
            cout << "No batches in the FEFO queue.\n";
            return;
        }
        priority_queue<ExpiryEntry, vector<ExpiryEntry>, CompareExpiry> temp = fefoQueue;
        while (!temp.empty()) {
            ExpiryEntry e = temp.top();
            temp.pop();
            int daysLeft = today.daysUntil(e.expiry);
            cout << "  " << e.sku << " | " << e.quantity << " units | expiry "
                 << e.expiry.toString() << " | ";
            if (daysLeft < 0) cout << "EXPIRED\n";
            else if (daysLeft <= soonDays) cout << "expires in " << daysLeft << " day(s)\n";
            else cout << "OK (" << daysLeft << " days left)\n";
        }
    }

    void displayAlerts(const Date& today) const {
        vector<ExpiryEntry> expired = getExpiredBatches(today);
        vector<ExpiryEntry> soon = getExpiringSoonBatches(today);

        cout << "Expired batches (" << expired.size() << "):\n";
        for (size_t i = 0; i < expired.size(); i++) {
            cout << "  " << expired[i].sku << " | " << expired[i].quantity
                 << " units | expired on " << expired[i].expiry.toString() << "\n";
        }
        cout << "Expiring within " << soonDays << " days (" << soon.size() << "):\n";
        for (size_t i = 0; i < soon.size(); i++) {
            cout << "  " << soon[i].sku << " | " << soon[i].quantity
                 << " units | expires on " << soon[i].expiry.toString() << "\n";
        }
    }
};

#endif