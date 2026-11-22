

#include "Product.h"

Product::Product() {
    sku = "";
    name = "";
    costPrice = 0.0;
    sellingPrice = 0.0;
    shelfId = -1;
    spacePerUnit = 1;
}

Product::Product(std::string sku, std::string name, double costPrice, double sellingPrice, int shelfId, int spacePerUnit) {
    this->sku = sku;
    this->name = name;
    this->costPrice = costPrice;
    this->sellingPrice = sellingPrice;
    this->shelfId = shelfId;
    this->spacePerUnit = (spacePerUnit < 1) ? 1 : spacePerUnit;
}

std::string Product::getSku() const {
    return sku;
}

std::string Product::getName() const {
    return name;
}

double Product::getCostPrice() const {
    return costPrice;
}

double Product::getSellingPrice() const {
    return sellingPrice;
}

int Product::getShelfId() const {
    return shelfId;
}

int Product::getSpacePerUnit() const {
    return spacePerUnit;
}

void Product::setSpacePerUnit(int space) {
    if (space >= 1) {
        spacePerUnit = space;
    }
}

int Product::getSpaceUsed() const {
    return getTotalStock() * spacePerUnit;
}

void Product::setShelfId(int id) {
    if (id >= 0) {
        shelfId = id;
    }
}

void Product::setCostPrice(double price) {
    if (price >= 0.0) {
        costPrice = price;
    }
}

void Product::setSellingPrice(double price) {
    if (price >= 0.0) {
        sellingPrice = price;
    }
}

void Product::setName(const std::string& newName) {
    if (!newName.empty()) {
        name = newName;
    }
}

std::vector<Batch> Product::getBatches() const {
    return batches;
}

void Product::clearBatches() {
    batches.clear();
}

int Product::getTotalStock() const {
    int total = 0;
    for (int i = 0; i < (int)batches.size(); i++) {
        if (batches[i].quantity > 0) {
            total = total + batches[i].quantity;
        }
    }
    return total;
}

bool Product::addBatch(int quantity, int expiryDate) {
    
    if (quantity <= 0 || expiryDate <= 0) {
        return false;
    }
    Batch newBatch;
    newBatch.quantity = quantity;
    newBatch.expiryDate = expiryDate;
    batches.push_back(newBatch);
    
    for (int i = 0; i < (int)batches.size() - 1; i++) {
        for (int j = 0; j < (int)batches.size() - i - 1; j++) {
            if (batches[j].expiryDate > batches[j + 1].expiryDate) {
                Batch temp = batches[j];
                batches[j] = batches[j + 1];
                batches[j + 1] = temp;
            }
        }
    }
    return true;
}

bool Product::reduceStock(int quantity) {
    
    if (quantity <= 0 || quantity > getTotalStock()) {
        return false;
    }
    int remainingToDeduct = quantity;
    for (int i = 0; i < (int)batches.size(); i++) {
        if (batches[i].quantity <= remainingToDeduct) {
            remainingToDeduct = remainingToDeduct - batches[i].quantity;
            batches[i].quantity = 0;
        } else {
            batches[i].quantity = batches[i].quantity - remainingToDeduct;
            remainingToDeduct = 0;
            break;
        }
    }
    
    std::vector<Batch> activeBatches;
    for (int i = 0; i < (int)batches.size(); i++) {
        if (batches[i].quantity > 0) {
            activeBatches.push_back(batches[i]);
        }
    }
    batches = activeBatches;
    return true;
}
