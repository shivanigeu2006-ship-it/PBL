// Supplier.h - purchasing details: lead time (days) and default safety stock (units).
#pragma once
#include <string>
#include "Utils.h"

class Supplier {
    std::string id_, name_, phone_;
    int leadTime_ = 1;
    int safetyStock_ = 0;

public:
    Supplier() = default;
    Supplier(std::string id, std::string name, std::string phone, int leadTime, int safetyStock)
        : id_(std::move(id)), name_(std::move(name)), phone_(std::move(phone)),
          leadTime_(leadTime), safetyStock_(safetyStock) {}

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& phone() const { return phone_; }
    int leadTime() const { return leadTime_; }
    int safetyStock() const { return safetyStock_; }

    void setName(const std::string& v) { name_ = v; }
    void setPhone(const std::string& v) { phone_ = v; }
    void setLeadTime(int v) { leadTime_ = v; }
    void setSafetyStock(int v) { safetyStock_ = v; }

    json toJson() const {
        return {{"id", id_}, {"name", name_}, {"phone", phone_}, {"leadTime", leadTime_},
                {"safetyStock", safetyStock_}};
    }
    static Supplier fromJson(const json& j) {
        return Supplier(j.at("id"), j.at("name"), j.value("phone", ""), j.at("leadTime"),
                        j.value("safetyStock", 0));
    }
};
