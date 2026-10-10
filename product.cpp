// ============================================================
// Product.cpp  -  Module 1: Product (with a small Date helper)
// A product is identified by a unique SKU. Stock is kept in
// batches, and every batch has its own quantity and expiry date.
// This makes FEFO (First Expiry First Out) possible.
// ============================================================
#ifndef PRODUCT_CPP
#define PRODUCT_CPP

#include <iostream>
#include <string>
#include <vector>
#include <algorithm>
using namespace std;

// ---------------- Date (small helper class) ----------------
class Date {
private:
    int day, month, year;

    static bool isLeapYear(int y) {
        return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
    }

    static int daysInMonth(int m, int y) {
        if (m == 2) return isLeapYear(y) ? 29 : 28;
        if (m == 4 || m == 6 || m == 9 || m == 11) return 30;
        return 31;
    }

    static string twoDigits(int n) {
        if (n < 10) return "0" + to_string(n);
        return to_string(n);
    }

public:
    Date() : day(1), month(1), year(2000) {}
    Date(int d, int m, int y) : day(d), month(m), year(y) {}

    bool isValid() const {
        if (year < 1 || month < 1 || month > 12 || day < 1) return false;
        return day <= daysInMonth(month, year);
    }

    // Converts the date into a single number of days so that
    // dates can be compared and subtracted easily.
    int toDayNumber() const {
        int y = year - 1;
        int total = y * 365 + y / 4 - y / 100 + y / 400;
        for (int m = 1; m < month; m++) {
            total += daysInMonth(m, year);
        }
        total += day;
        return total;
    }

    // Number of days from this date to the other date
    int daysUntil(const Date& other) const {
        return other.toDayNumber() - toDayNumber();
    }

    bool isBefore(const Date& other) const {
        return toDayNumber() < other.toDayNumber();
    }

    string toString() const {
        return to_string(year) + "-" + twoDigits(month) + "-" + twoDigits(day);
    }
};

// ---------------- Batch (one delivery of a product) ----------------
struct Batch {
    int quantity;
    Date expiry;

    Batch() : quantity(0) {}
    Batch(int q, const Date& e) : quantity(q), expiry(e) {}
};

// ---------------- Product ----------------
class Product {
private:
    string sku;
    string name;
    double costPrice;
    double sellingPrice;
    string shelfId;          // shelf location ("" means not assigned yet)
    vector<Batch> batches;   // batch-wise stock and expiry details

public:
    Product() : sku(""), name(""), costPrice(0), sellingPrice(0), shelfId("") {}

    Product(const string& skuCode, const string& productName,
            double cost, double selling)
        : sku(skuCode), name(productName), costPrice(0), sellingPrice(0), shelfId("") {
        setCostPrice(cost);
        setSellingPrice(selling);
    }

    // ----- getters -----
    string getSku() const { return sku; }
    string getName() const { return name; }
    double getCostPrice() const { return costPrice; }
    double getSellingPrice() const { return sellingPrice; }
    string getShelfId() const { return shelfId; }
    const vector<Batch>& getBatches() const { return batches; }

    // ----- setters (with validation) -----
    bool setCostPrice(double price) {
        if (price < 0) {
            cout << "Error: cost price cannot be negative.\n";
            return false;
        }
        costPrice = price;
        return true;
    }

    bool setSellingPrice(double price) {
        if (price < 0) {
            cout << "Error: selling price cannot be negative.\n";
            return false;
        }
        sellingPrice = price;
        return true;
    }

    void setShelfId(const string& id) { shelfId = id; }

    // ----- stock handling -----

    // Adds a new batch (a new delivery) with its own expiry date
    bool addBatch(int quantity, const Date& expiry) {
        if (quantity <= 0) {
            cout << "Error: batch quantity must be greater than 0.\n";
            return false;
        }
        if (!expiry.isValid()) {
            cout << "Error: invalid expiry date.\n";
            return false;
        }
        batches.push_back(Batch(quantity, expiry));
        return true;
    }

    // Total units in stock (all batches)
    int getTotalStock() const {
        int total = 0;
        for (size_t i = 0; i < batches.size(); i++) {
            total += batches[i].quantity;
        }
        return total;
    }

    // Units that are not expired on the given date
    int getUnexpiredStock(const Date& today) const {
        int total = 0;
        for (size_t i = 0; i < batches.size(); i++) {
            if (!batches[i].expiry.isBefore(today)) {
                total += batches[i].quantity;
            }
        }
        return total;
    }

    // Removes units using FEFO: the batch with the earliest expiry
    // date is used first. Expired batches are never sold.
    bool removeStockFEFO(int units, const Date& today) {
        if (units <= 0) {
            cout << "Error: units to sell must be greater than 0.\n";
            return false;
        }
        if (!today.isValid()) {
            cout << "Error: invalid date.\n";
            return false;
        }
        if (getUnexpiredStock(today) < units) {
            cout << "Error: not enough unexpired stock for SKU " << sku << ".\n";
            return false;
        }

        int remaining = units;
        while (remaining > 0) {
            // find the unexpired batch with the earliest expiry date
            int best = -1;
            for (size_t i = 0; i < batches.size(); i++) {
                if (batches[i].quantity > 0 && !batches[i].expiry.isBefore(today)) {
                    if (best == -1 || batches[i].expiry.isBefore(batches[best].expiry)) {
                        best = (int)i;
                    }
                }
            }
            if (best == -1) break;   // safety check, should not happen

            int take = min(remaining, batches[best].quantity);
            batches[best].quantity -= take;
            remaining -= take;

            if (batches[best].quantity == 0) {
                batches.erase(batches.begin() + best);
            }
        }
        return true;
    }

    void display() const {
        cout << "SKU: " << sku << " | Name: " << name
             << " | Cost: " << costPrice << " | Price: " << sellingPrice
             << " | Shelf: " << (shelfId.empty() ? "Not assigned" : shelfId)
             << " | Total Stock: " << getTotalStock() << "\n";
        for (size_t i = 0; i < batches.size(); i++) {
            cout << "    Batch " << (i + 1) << ": " << batches[i].quantity
                 << " units, expiry " << batches[i].expiry.toString() << "\n";
        }
    }
};

#endif