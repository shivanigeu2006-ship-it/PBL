
#pragma once
#include <vector>
#include <string>
#include "Inventory.h"

struct Sale {
    std::string id, sku, name;
    int units = 0;
    long day = 0;
    double unitPrice = 0, unitCost = 0;
    double revenue() const { return unitPrice * units; }
    double profit() const { return (unitPrice - unitCost) * units; }
};

inline void to_json(json& j, const Sale& s) {
    j = {{"id", s.id}, {"sku", s.sku}, {"name", s.name}, {"units", s.units},
         {"date", util::formatDate(s.day)}, {"unitPrice", s.unitPrice}, {"unitCost", s.unitCost},
         {"revenue", util::r2(s.revenue())}, {"profit", util::r2(s.profit())}};
}
inline void from_json(const json& j, Sale& s) {
    s.id = j.at("id").get<std::string>();
    s.sku = j.at("sku").get<std::string>();
    s.name = j.value("name", "");
    s.units = j.at("units").get<int>();
    util::parseDate(j.at("date").get<std::string>(), s.day);
    s.unitPrice = j.at("unitPrice").get<double>();
    s.unitCost = j.at("unitCost").get<double>();
}

class SalesRecord {
    std::vector<Sale> sales_;
    int counter_ = 0;

public:
    struct Result {
        bool ok = false;
        std::string error;
        Sale sale;
    };

    
    Result record(Inventory& inv, const std::string& sku, int units, long day) {
        Result r;
        Product* p = inv.find(sku);
        if (!p) { r.error = "Unknown SKU: " + sku; return r; }
        if (units <= 0) { r.error = "Units must be at least 1"; return r; }
        int available = p->sellableStock(day);
        if (units > available) {
            r.error = "Only " + std::to_string(available) + " sellable unit(s) of " + p->name() + " in stock";
            return r;
        }
        p->sell(units, day);
        Sale s;
        char buf[16];
        std::snprintf(buf, sizeof buf, "S%05d", ++counter_);
        s.id = buf; s.sku = sku; s.name = p->name(); s.units = units; s.day = day;
        s.unitPrice = p->price(); s.unitCost = p->cost();
        sales_.push_back(s);
        r.ok = true; r.sale = s;
        return r;
    }

    const std::vector<Sale>& all() const { return sales_; }
    void clear() { sales_.clear(); counter_ = 0; }
    void load(std::vector<Sale> v, int counter) { sales_ = std::move(v); counter_ = counter; }
    int counter() const { return counter_; }

   
    int unitsSold(const std::string& sku, long from, long to) const {
        int n = 0;
        for (auto& s : sales_) if (s.sku == sku && s.day >= from && s.day <= to) n += s.units;
        return n;
    }
  
    long firstDay(long fallback) const {
        long d = fallback;
        for (auto& s : sales_) d = std::min(d, s.day);
        return d;
    }
};
