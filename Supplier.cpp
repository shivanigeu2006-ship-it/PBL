// ============================================================
// Supplier.cpp  -  Module 5: Supplier
// Stores supplier details, lead time (in days) and safety stock.
// These values are used later by ReorderManager.
// ============================================================
#ifndef SUPPLIER_CPP
#define SUPPLIER_CPP

#include <iostream>
#include <string>
using namespace std;

class Supplier {
private:
    string supplierId;
    string name;
    string contact;
    int leadTimeDays;   // days the supplier takes to deliver
    int safetyStock;    // extra units kept as a buffer

public:
    Supplier() : supplierId(""), name(""), contact(""), leadTimeDays(0), safetyStock(0) {}

    Supplier(const string& id, const string& supplierName, const string& contactInfo,
             int leadTime, int safety)
        : supplierId(id), name(supplierName), contact(contactInfo),
          leadTimeDays(0), safetyStock(0) {
        setLeadTimeDays(leadTime);
        setSafetyStock(safety);
    }

    // ----- getters -----
    string getSupplierId() const { return supplierId; }
    string getName() const { return name; }
    string getContact() const { return contact; }
    int getLeadTimeDays() const { return leadTimeDays; }
    int getSafetyStock() const { return safetyStock; }

    // ----- setters (with validation) -----
    bool setLeadTimeDays(int days) {
        if (days < 0) {
            cout << "Error: lead time cannot be negative.\n";
            return false;
        }
        leadTimeDays = days;
        return true;
    }

    bool setSafetyStock(int units) {
        if (units < 0) {
            cout << "Error: safety stock cannot be negative.\n";
            return false;
        }
        safetyStock = units;
        return true;
    }

    void display() const {
        cout << "Supplier " << supplierId << " | " << name << " | Contact: " << contact
             << " | Lead time: " << leadTimeDays << " days"
             << " | Safety stock: " << safetyStock << "\n";
    }
};

#endif