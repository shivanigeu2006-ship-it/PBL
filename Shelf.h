#ifndef SHELF_H
#define SHELF_H
#include <iostream>
#include <string>
// Report struct holding shelf usage and availability metrics
struct ShelfReport {
    int shelfId;
    std::string name;
    int capacity;
    int usedSpace;
    int freeSpace;
    double utilizationPercent;
};
// Shelf class manages capacity, used space, and space verification
class Shelf {
private:
    int shelfId;
    std::string name; // kind of shelf, e.g. "Dairy", "Frozen", "General"
    int capacity;
    int usedSpace;
public:
    Shelf();
    Shelf(int id, std::string name, int capacity);
    int getShelfId() const;
    std::string getName() const;
    int getCapacity() const;
    int getUsedSpace() const;
    int freeSpace() const;
    bool canFit(int quantity) const;
    bool addUsed(int quantity);
    bool removeUsed(int quantity);
    void setCapacity(int cap);
    void setUsedSpace(int used);
};
#endif
