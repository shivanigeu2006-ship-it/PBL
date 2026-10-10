// ============================================================
// SalesRecord.cpp  -  Module 4: SalesRecord
// Records units sold and updates stock after every sale.
// A sale is stored only if the stock update succeeds.
// ============================================================
#ifndef SALESRECORD_CPP
#define SALESRECORD_CPP

#include <iostream>
#include <string>
#include <vector>
#include "Inventory.cpp"
using namespace std;

struct Sale {
    string sku;
    int units;
    Date date;

    Sale() : sku(""), units(0) {}
    Sale(const string& s, int u, const Date& d) : sku(s), units(u), date(d) {}
};

class SalesRecord {
private:
    vector<Sale> sales;   // array of all sales

public:
    // Records a sale and reduces stock in the inventory.
    // Returns true only if the sale was valid and stock was updated.
    bool recordSale(Inventory& inventory, const string& sku, int units, const Date& date) {
        if (units <= 0) {
            cout << "Error: units sold must be greater than 0.\n";
            return false;
        }
        if (!date.isValid()) {
            cout << "Error: invalid sale date.\n";
            return false;
        }
        if (!inventory.productExists(sku)) {
            cout << "Error: SKU " << sku << " not found.\n";
            return false;
        }
        if (!inventory.sellStock(sku, units, date)) {
            return false;   // not enough stock (message already shown)
        }
        sales.push_back(Sale(sku, units, date));
        return true;
    }

    // Total units sold for one SKU
    int getTotalUnitsSold(const string& sku) const {
        int total = 0;
        for (size_t i = 0; i < sales.size(); i++) {
            if (sales[i].sku == sku) {
                total += sales[i].units;
            }
        }
        return total;
    }

    int getSalesCount() const {
        return (int)sales.size();
    }

    void displayAll() const {
        if (sales.empty()) {
            cout << "No sales recorded.\n";
            return;
        }
        for (size_t i = 0; i < sales.size(); i++) {
            cout << "Sale " << (i + 1) << ": SKU " << sales[i].sku
                 << ", units " << sales[i].units
                 << ", date " << sales[i].date.toString() << "\n";
        }
    }
};

#endif