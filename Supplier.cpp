

#include "Supplier.h"

Supplier::Supplier() {
    supplierName = "";
    sku = "";
    leadTime = 3;
    safetyStock = 10;
}

Supplier::Supplier(std::string name, std::string sku, int leadTime, int safetyStock) {
    this->supplierName = name;
    this->sku = sku;
    if (leadTime < 1) {
        this->leadTime = 1;
    } else {
        this->leadTime = leadTime;
    }
    if (safetyStock < 0) {
        this->safetyStock = 0;
    } else {
        this->safetyStock = safetyStock;
    }
}

std::string Supplier::getSupplierName() const {
    return supplierName;
}

void Supplier::setSupplierName(const std::string& name) {
    supplierName = name;
}

std::string Supplier::getSku() const {
    return sku;
}

void Supplier::setSku(const std::string& sku) {
    this->sku = sku;
}

int Supplier::getLeadTime() const {
    return leadTime;
}

void Supplier::setLeadTime(int days) {
    if (days >= 1) {
        leadTime = days;
    }
}

int Supplier::getSafetyStock() const {
    return safetyStock;
}

void Supplier::setSafetyStock(int stock) {
    if (stock >= 0) {
        safetyStock = stock;
    }
}
