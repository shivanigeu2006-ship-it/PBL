// Product.h - a product identified by SKU. Stock is held in batches, each with
// its own quantity and expiry date, so FEFO always sells the earliest expiry first.
#pragma once
#include <string>
#include <vector>
#include <algorithm>
#include "Utils.h"

struct Batch {
    std::string id;
    int qty = 0;
    long expiry = util::NO_EXPIRY;
    long received = 0;
};

inline void to_json(json& j, const Batch& b) {
    j = {{"id", b.id}, {"qty", b.qty}, {"expiry", util::formatDate(b.expiry)},
         {"received", util::formatDate(b.received)}};
}
inline void from_json(const json& j, Batch& b) {
    b.id = j.at("id").get<std::string>();
    b.qty = j.at("qty").get<int>();
    util::parseDate(j.at("expiry").get<std::string>(), b.expiry);
    util::parseDate(j.at("received").get<std::string>(), b.received);
}

class Product {
    std::string sku_, name_, category_ = "General", shelfId_, supplierId_;
    double cost_ = 0, price_ = 0;
    int target_ = 0;
    int safety_ = -1;  // -1 => use the supplier's default safety stock
    int counter_ = 0;
    std::vector<Batch> batches_;

public:
    Product() = default;
    Product(std::string sku, std::string name, std::string category, double cost, double price, int target)
        : sku_(std::move(sku)), name_(std::move(name)), category_(std::move(category)),
          cost_(cost), price_(price), target_(target) {}

    // ---- getters ----
    const std::string& sku() const { return sku_; }
    const std::string& name() const { return name_; }
    const std::string& category() const { return category_; }
    const std::string& shelfId() const { return shelfId_; }
    const std::string& supplierId() const { return supplierId_; }
    double cost() const { return cost_; }
    double price() const { return price_; }
    int target() const { return target_; }
    int safety() const { return safety_; }
    const std::vector<Batch>& batches() const { return batches_; }

    // ---- setters ----
    void setName(const std::string& v) { name_ = v; }
    void setCategory(const std::string& v) { category_ = v; }
    void setCost(double v) { cost_ = v; }
    void setPrice(double v) { price_ = v; }
    void setTarget(int v) { target_ = v; }
    void setSafety(int v) { safety_ = v; }
    void setShelf(const std::string& v) { shelfId_ = v; }
    void setSupplier(const std::string& v) { supplierId_ = v; }

    // ---- stock ----
    Batch& addBatch(int qty, long expiry, long received) {
        char buf[16];
        std::snprintf(buf, sizeof buf, "-B%03d", ++counter_);
        batches_.push_back({sku_ + buf, qty, expiry, received});
        return batches_.back();
    }
    int totalStock() const { int s = 0; for (auto& b : batches_) s += b.qty; return s; }
    int expiredStock(long t) const { int s = 0; for (auto& b : batches_) if (b.expiry < t) s += b.qty; return s; }
    int sellableStock(long t) const { return totalStock() - expiredStock(t); }

    // FEFO: take units from non-expired batches, earliest expiry first.
    // Returns the units actually taken. Empty batches are removed.
    int sell(int units, long t) {
        std::stable_sort(batches_.begin(), batches_.end(),
                         [](const Batch& a, const Batch& b) { return a.expiry < b.expiry; });
        int taken = 0;
        for (auto& b : batches_) {
            if (taken >= units) break;
            if (b.expiry < t || b.qty <= 0) continue;
            int n = std::min(b.qty, units - taken);
            b.qty -= n;
            taken += n;
        }
        batches_.erase(std::remove_if(batches_.begin(), batches_.end(),
                                      [](const Batch& b) { return b.qty <= 0; }), batches_.end());
        return taken;
    }

    // Removes every expired batch (write-off) and returns what was removed.
    std::vector<Batch> discardExpired(long t) {
        std::vector<Batch> gone;
        for (auto& b : batches_) if (b.expiry < t) gone.push_back(b);
        batches_.erase(std::remove_if(batches_.begin(), batches_.end(),
                                      [t](const Batch& b) { return b.expiry < t; }), batches_.end());
        return gone;
    }

    // ---- profit helpers ----
    double profitPerUnit() const { return price_ - cost_; }
    double marginPct() const { return price_ > 0 ? (price_ - cost_) / price_ * 100.0 : 0.0; }

    // ---- persistence ----
    json toJson() const {
        return {{"sku", sku_}, {"name", name_}, {"category", category_}, {"cost", cost_}, {"price", price_},
                {"target", target_}, {"safety", safety_}, {"shelfId", shelfId_}, {"supplierId", supplierId_},
                {"counter", counter_}, {"batches", batches_}};
    }
    static Product fromJson(const json& j) {
        Product p(j.at("sku"), j.at("name"), j.value("category", "General"), j.at("cost"), j.at("price"),
                  j.value("target", 0));
        p.safety_ = j.value("safety", -1);
        p.shelfId_ = j.value("shelfId", "");
        p.supplierId_ = j.value("supplierId", "");
        p.counter_ = j.value("counter", 0);
        p.batches_ = j.value("batches", std::vector<Batch>{});
        return p;
    }
};
