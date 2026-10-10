// ============================================================
// Shelf.cpp  -  Module 2: Shelf
// Tracks the capacity, used space and free space of one shelf.
// (Automatic shelf assignment belongs to the pending ShelfManager.)
// ============================================================
#ifndef SHELF_CPP
#define SHELF_CPP

#include <iostream>
#include <string>
using namespace std;

class Shelf {
private:
    string shelfId;
    int capacity;     // maximum units the shelf can hold
    int usedSpace;    // units currently placed on the shelf

public:
    Shelf() : shelfId(""), capacity(0), usedSpace(0) {}

    Shelf(const string& id, int cap) : shelfId(id), capacity(0), usedSpace(0) {
        setCapacity(cap);
    }

    // ----- getters -----
    string getShelfId() const { return shelfId; }
    int getCapacity() const { return capacity; }
    int getUsedSpace() const { return usedSpace; }
    int getFreeSpace() const { return capacity - usedSpace; }
    bool isFull() const { return usedSpace == capacity; }

    // ----- setter with validation -----
    bool setCapacity(int cap) {
        if (cap <= 0) {
            cout << "Error: shelf capacity must be greater than 0.\n";
            return false;
        }
        if (cap < usedSpace) {
            cout << "Error: new capacity is less than the space already used.\n";
            return false;
        }
        capacity = cap;
        return true;
    }

    // Place units on the shelf. Rejected if there is not enough free space.
    bool addUnits(int units) {
        if (units <= 0) {
            cout << "Error: units must be greater than 0.\n";
            return false;
        }
        if (units > getFreeSpace()) {
            cout << "Error: shelf " << shelfId << " does not have enough free space.\n";
            return false;
        }
        usedSpace += units;
        return true;
    }

    // Take units off the shelf (for example after a sale or a move)
    bool removeUnits(int units) {
        if (units <= 0) {
            cout << "Error: units must be greater than 0.\n";
            return false;
        }
        if (units > usedSpace) {
            cout << "Error: cannot remove more units than are on the shelf.\n";
            return false;
        }
        usedSpace -= units;
        return true;
    }

    void display() const {
        cout << "Shelf " << shelfId << " | Capacity: " << capacity
             << " | Used: " << usedSpace << " | Free: " << getFreeSpace() << "\n";
    }
};

#endif