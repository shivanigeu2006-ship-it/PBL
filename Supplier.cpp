// ============================================================================
// Module: Supplier.cpp (Module 5/11)
// Concept: OOP Encapsulation for Procurement Parameters
// Formulas / Role: Supplies lead time and safety stock used in Reorder Point formula
// ============================================================================
#include "Supplier.h"
// Default constructor initializes default values
Supplier::Supplier() {
    supplierName = "";
    sku = "";
    leadTime = 3;
    safetyStock = 10;
}
// Parameterized constructor validates lead time and safety stock
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
// Returns the supplier name
std::string Supplier::getSupplierName() const {
    return supplierName;
}
// Updates supplier name
void Supplier::setSupplierName(const std::string& name) {
    supplierName = name;
}
// Returns the SKU assigned to this supplier
std::string Supplier::getSku() const {
    return sku;
}
// Updates the SKU
void Supplier::setSku(const std::string& sku) {
    this->sku = sku;
}
// Returns lead time in days
int Supplier::getLeadTime() const {
    return leadTime;
}
// Updates lead time in days
void Supplier::setLeadTime(int days) {
    if (days >= 1) {
        leadTime = days;
    }
}
// Returns safety stock quantity
int Supplier::getSafetyStock() const {
    return safetyStock;
}
// Updates safety stock quantity
void Supplier::setSafetyStock(int stock) {
    if (stock >= 0) {
        safetyStock = stock;
    }
}
