

#include "Shelf.h"

Shelf::Shelf() {
    shelfId = -1;
    name = "";
    capacity = 0;
    usedSpace = 0;
}

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

int Shelf::getShelfId() const {
    return shelfId;
}

std::string Shelf::getName() const {
    return name;
}

int Shelf::getCapacity() const {
    return capacity;
}

int Shelf::getUsedSpace() const {
    return usedSpace;
}

void Shelf::setCapacity(int cap) {
    if (cap >= 0) {
        capacity = cap;
    }
}

void Shelf::setUsedSpace(int used) {
    if (used >= 0 && used <= capacity) {
        usedSpace = used;
    }
}

int Shelf::freeSpace() const {
    if (capacity >= usedSpace) {
        return capacity - usedSpace;
    }
    return 0;
}

bool Shelf::canFit(int quantity) const {
    if (quantity < 0) {
        return false;
    }
    return (usedSpace + quantity <= capacity);
}

bool Shelf::addUsed(int quantity) {
    if (quantity <= 0 || !canFit(quantity)) {
        return false;
    }
    usedSpace = usedSpace + quantity;
    return true;
}

bool Shelf::removeUsed(int quantity) {
    if (quantity <= 0 || quantity > usedSpace) {
        return false;
    }
    usedSpace = usedSpace - quantity;
    return true;
}
