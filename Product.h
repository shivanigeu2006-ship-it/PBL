#ifndef PRODUCT_H
#define PRODUCT_H
#include <iostream>
#include <string>
#include <vector>

inline bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

inline bool isLeap(int y) {
    return (y % 4 == 0 && y % 100 != 0) || (y % 400 == 0);
}

inline int dateToDays(int yyyymmdd) {
    int year  = yyyymmdd / 10000;
    int month = (yyyymmdd % 10000) / 100;
    int day   = yyyymmdd % 100;

    int daysInMonth[] = {31,28,31,30,31,30,31,31,30,31,30,31};
    int total = 0;

    for (int y = 2000; y < year; y++)          // full years since 2000
        total += isLeap(y) ? 366 : 365;

    for (int m = 1; m < month; m++) {          // full months this year
        total += daysInMonth[m - 1];
        if (m == 2 && isLeap(year)) total++;   // leap day
    }

    return total + day;
}
inline int daysDifference(int fromDate, int toDate) {
    return dateToDays(toDate) - dateToDays(fromDate);
}

inline std::string formatDate(int yyyymmdd) {
    int year = yyyymmdd / 10000;
    int month = (yyyymmdd % 10000) / 100;
    int day = yyyymmdd % 100;
    std::string m = (month < 10 ? "0" : "") + std::to_string(month);
    std::string d = (day < 10 ? "0" : "") + std::to_string(day);
    return std::to_string(year) + "-" + m + "-" + d;
}

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

struct Batch {
    int quantity;
    int expiryDate; 
};

struct ProductReport {
    std::string sku;
    std::string name;
    double costPrice;
    double sellingPrice;
    int shelfId;
    std::string shelfName; 
    int spacePerUnit;
    int totalStock;
    std::vector<Batch> batches;
};

class Product {
private:
    std::string sku;
    std::string name;
    double costPrice;
    double sellingPrice;
    int shelfId;
    int spacePerUnit; 
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
    int getSpaceUsed() const; 
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
