// ExpiryManager.h - finds expired and soon-to-expire batches and orders them by FEFO
// (First Expired, First Out) using a min-heap priority_queue keyed on expiry date.
#pragma once
#include <queue>
#include <vector>
#include "Inventory.h"

struct ExpiryItem {
    std::string sku, name, batchId, expiry, status, shelfId;
    int qty = 0;
    long expiryDay = 0;
    int daysLeft = 0;
    double valueAtRisk = 0;  // cost price x quantity
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ExpiryItem, sku, name, batchId, expiry, status, shelfId, qty, expiryDay,
                                   daysLeft, valueAtRisk)

class ExpiryManager {
    struct Node {
        long expiry;
        std::string sku, batchId;
        int qty;
    };
    struct Later {  // makes priority_queue a MIN-heap on expiry (earliest expiry on top)
        bool operator()(const Node& a, const Node& b) const {
            if (a.expiry != b.expiry) return a.expiry > b.expiry;
            return a.batchId > b.batchId;
        }
    };

public:
    static const int CRITICAL_DAYS = 1;

    // Every perishable batch in FEFO order (earliest expiry first).
    // status: expired | critical (<=1 day) | soon (<= soonDays) | ok
    std::vector<ExpiryItem> fefoQueue(const Inventory& inv, long today, int soonDays) const {
        std::priority_queue<Node, std::vector<Node>, Later> pq;
        for (auto* p : inv.all())
            for (auto& b : p->batches())
                if (b.qty > 0 && b.expiry < util::NO_EXPIRY) pq.push({b.expiry, p->sku(), b.id, b.qty});

        std::vector<ExpiryItem> out;
        while (!pq.empty()) {  // O(log n) per removal
            Node n = pq.top();
            pq.pop();
            const Product* p = inv.find(n.sku);
            ExpiryItem e;
            e.sku = n.sku; e.name = p->name(); e.batchId = n.batchId; e.qty = n.qty;
            e.shelfId = p->shelfId();
            e.expiryDay = n.expiry; e.expiry = util::formatDate(n.expiry);
            e.daysLeft = (int)(n.expiry - today);
            e.valueAtRisk = util::r2(p->cost() * n.qty);
            e.status = n.expiry < today ? "expired"
                       : e.daysLeft <= CRITICAL_DAYS ? "critical"
                       : e.daysLeft <= soonDays ? "soon" : "ok";
            out.push_back(e);
        }
        return out;
    }

    std::vector<ExpiryItem> expired(const Inventory& inv, long today) const {
        std::vector<ExpiryItem> v;
        for (auto& e : fefoQueue(inv, today, 0)) if (e.status == "expired") v.push_back(e);
        return v;
    }
    std::vector<ExpiryItem> expiringSoon(const Inventory& inv, long today, int soonDays) const {
        std::vector<ExpiryItem> v;
        for (auto& e : fefoQueue(inv, today, soonDays)) if (e.status == "critical" || e.status == "soon") v.push_back(e);
        return v;
    }
};
