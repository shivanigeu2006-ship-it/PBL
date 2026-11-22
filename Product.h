#ifndef PRODUCT_H
#define PRODUCT_H
#include <iostream>
#include <string>
#include <vector>
// Helper function: converts YYYYMMDD (e.g. 20261122) into total days
// Uses civil calendar calculation without needing <chrono>
inline int dateToDays(int yyyymmdd) {
    int year = yyyymmdd / 10000;
    int month = (yyyymmdd % 10000) / 100;
    int day = yyyymmdd % 100;
    if (month <= 2) {
        year = year - 1;
        month = month + 12;
    }
    return 365 * year + year / 4 - year / 100 + year / 400 + (153 * (month + 1)) / 5 + day;
}
// Helper function: returns difference in days (toDate - fromDate)
inline int daysDifference(int fromDate, int toDate) {
    return dateToDays(toDate) - dateToDays(fromDate);
}
// Helper function: formats YYYYMMDD (e.g. 20260815) as "2026-08-15"
inline std::string formatDate(int yyyymmdd) {
    int year = yyyymmdd / 10000;
    int month = (yyyymmdd % 10000) / 100;
    int day = yyyymmdd % 100;
    std::string m = (month < 10 ? "0" : "") + std::to_string(month);
    std::string d = (day < 10 ? "0" : "") + std::to_string(day);
    return std::to_string(year) + "-" + m + "-" + d;
}
// Helper function: parses "2026-08-15" (or "20260815") into YYYYMMDD; returns 0 if invalid
inline int parseDate(const std::string& text) {
    std::string digits = "";
    for (int i = 0; i < (int)text.size(); i++) {
        if (text[i] >= '0' && text[i] <= '9') {
            digits += text[i];
        } else if (text[i] != '-' && text[i] != '/') {
            return 0;
        }
    }
    if (digits.size() != 8) {
        return 0;
    }
    int value = std::stoi(digits);
    int year = value / 10000;
    int month = (value % 10000) / 100;
    int day = value % 100;
    int daysInMonth[] = {31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31};
    bool leap = (year % 4 == 0 && year % 100 != 0) || (year % 400 == 0);
    if (year < 1900 || month < 1 || month > 12 || day < 1) {
        return 0;
    }
    int maxDay = daysInMonth[month - 1] + ((month == 2 && leap) ? 1 : 0);
    if (day > maxDay) {
        return 0;
    }
    return value;
}
// Represents an individual delivery batch with quantity and expiry date
struct Batch {
    int quantity;
    int expiryDate; // Format: YYYYMMDD
};
// Report struct holding product details and batch breakdown
struct ProductReport {
    std::string sku;
    std::string name;
    double costPrice;
    double sellingPrice;
    int shelfId;
    std::string shelfName; // name of the shelf, e.g. "Dairy"
    int spacePerUnit;
    int totalStock;
    std::vector<Batch> batches;
};
// Product class manages SKU, prices, shelf placement, and batches
class Product {
private:
    std::string sku;
    std::string name;
    double costPrice;
    double sellingPrice;
    int shelfId;
    int spacePerUnit; // shelf space one unit occupies (1 = small packet)
    std::vector<Batch> batches;
public:
    Product();
    Product(std::string sku, std::string name, double costPrice, double sellingPrice, int shelfId, int spacePerUnit = 1);
    std::string getSku() const;
    std::string getName() const;
    double getCostPrice() const;
    double getSellingPrice() const;
    int getShelfId() const;
    int getSpacePerUnit() const;
    int getSpaceUsed() const; // totalStock * spacePerUnit
    int getTotalStock() const;
    std::vector<Batch> getBatches() const;
    void setShelfId(int id);
    void setSpacePerUnit(int space);
    void setCostPrice(double price);
    void setSellingPrice(double price);
    void setName(const std::string& newName);
    bool addBatch(int quantity, int expiryDate);
    bool reduceStock(int quantity);
    void clearBatches();
};
#endif
