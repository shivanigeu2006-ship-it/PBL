#ifndef SUPPLIER_H
#define SUPPLIER_H
#include <iostream>
#include <string>
// Supplier class stores lead time and safety stock for a specific SKU
class Supplier {
private:
    std::string supplierName;
    std::string sku;
    int leadTime;    // Lead time in days
    int safetyStock; // Buffer stock in units
public:
    Supplier();
    Supplier(std::string name, std::string sku, int leadTime, int safetyStock);
    std::string getSupplierName() const;
    void setSupplierName(const std::string& name);
    std::string getSku() const;
    void setSku(const std::string& sku);
    int getLeadTime() const;
    void setLeadTime(int days);
    int getSafetyStock() const;
    void setSafetyStock(int stock);
};
#endif
