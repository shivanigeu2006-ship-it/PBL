// ============================================================================
// Module: Shelf.cpp (Module 2/11)
// Concept: OOP Encapsulation for Shelf Space Management
// Formulas: freeSpace = capacity - usedSpace; canFit = (usedSpace + qty <= capacity)
// ============================================================================
#include "Shelf.h"
// Default constructor initializes empty shelf
Shelf::Shelf() {
    shelfId = -1;
    name = "";
    capacity = 0;
    usedSpace = 0;
}
// Parameterized constructor sets shelf ID and total capacity
Shelf::Shelf(int id, std::string name, int capacity) {
    this->shelfId = id;
    this->name = name;
    if (capacity < 0) {
        this->capacity = 0;
    } else {
        this->capacity = capacity;
    }
    this->usedSpace = 0;
}
// Returns the shelf identifier
int Shelf::getShelfId() const {
    return shelfId;
}
// Returns the shelf name (its kind, e.g. "Dairy")
std::string Shelf::getName() const {
    return name;
}
// Returns total capacity of shelf
int Shelf::getCapacity() const {
    return capacity;
}
// Returns currently occupied space
int Shelf::getUsedSpace() const {
    return usedSpace;
}
// Updates total capacity
void Shelf::setCapacity(int cap) {
    if (cap >= 0) {
        capacity = cap;
    }
}
// Updates used space directly
void Shelf::setUsedSpace(int used) {
    if (used >= 0 && used <= capacity) {
        usedSpace = used;
    }
}
// Calculates remaining free space on the shelf
int Shelf::freeSpace() const {
    if (capacity >= usedSpace) {
        return capacity - usedSpace;
    }
    return 0;
}
// Checks if an incoming quantity fits without exceeding capacity
bool Shelf::canFit(int quantity) const {
    if (quantity < 0) {
        return false;
    }
    return (usedSpace + quantity <= capacity);
}
// Adds used space to shelf if capacity allows
bool Shelf::addUsed(int quantity) {
    if (quantity <= 0 || !canFit(quantity)) {
        return false;
    }
    usedSpace = usedSpace + quantity;
    return true;
}
// Frees up space when stock is sold or moved
bool Shelf::removeUsed(int quantity) {
    if (quantity <= 0 || quantity > usedSpace) {
        return false;
    }
    usedSpace = usedSpace - quantity;
    return true;
}
