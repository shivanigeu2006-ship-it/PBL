// ============================================================================
// Module: Product.cpp (Module 1/11)
// Concept: OOP Encapsulation, Dynamic Batch Management using std::vector
// Data Structure: std::vector<Batch> sorted by expiryDate ascending (FEFO)
// Formula: Total Stock = sum of batch quantities; stock is reduced from earliest batch
// ============================================================================
#include "Product.h"
// Default constructor initializes empty product
Product::Product() {
    sku = "";
    name = "";
    costPrice = 0.0;
    sellingPrice = 0.0;
    shelfId = -1;
    spacePerUnit = 1;
}
// Parameterized constructor sets basic product info
Product::Product(std::string sku, std::string name, double costPrice, double sellingPrice, int shelfId, int spacePerUnit) {
    this->sku = sku;
    this->name = name;
    this->costPrice = costPrice;
    this->sellingPrice = sellingPrice;
    this->shelfId = shelfId;
    this->spacePerUnit = (spacePerUnit < 1) ? 1 : spacePerUnit;
}
// Returns the SKU identifier
std::string Product::getSku() const {
    return sku;
}
// Returns the product name
std::string Product::getName() const {
    return name;
}
// Returns unit cost price
double Product::getCostPrice() const {
    return costPrice;
}
// Returns unit selling price
double Product::getSellingPrice() const {
    return sellingPrice;
}
// Returns assigned shelf ID
int Product::getShelfId() const {
    return shelfId;
}
// Returns shelf space taken by one unit
int Product::getSpacePerUnit() const {
    return spacePerUnit;
}
// Updates shelf space per unit
void Product::setSpacePerUnit(int space) {
    if (space >= 1) {
        spacePerUnit = space;
    }
}
// Total shelf space used by all units of this product
int Product::getSpaceUsed() const {
    return getTotalStock() * spacePerUnit;
}
// Updates assigned shelf ID
void Product::setShelfId(int id) {
    if (id >= 0) {
        shelfId = id;
    }
}
// Updates cost price
void Product::setCostPrice(double price) {
    if (price >= 0.0) {
        costPrice = price;
    }
}
// Updates selling price
void Product::setSellingPrice(double price) {
    if (price >= 0.0) {
        sellingPrice = price;
    }
}
// Updates product name
void Product::setName(const std::string& newName) {
    if (!newName.empty()) {
        name = newName;
    }
}
// Returns all batch records
std::vector<Batch> Product::getBatches() const {
    return batches;
}
// Clears all batches
void Product::clearBatches() {
    batches.clear();
}
// Calculates total stock by adding quantities of all active batches
int Product::getTotalStock() const {
    int total = 0;
    for (int i = 0; i < (int)batches.size(); i++) {
        if (batches[i].quantity > 0) {
            total = total + batches[i].quantity;
        }
    }
    return total;
}
// Adds a new delivery batch and maintains FEFO order (earliest expiry first)
bool Product::addBatch(int quantity, int expiryDate) {
    // Validate positive inputs
    if (quantity <= 0 || expiryDate <= 0) {
        return false;
    }
    Batch newBatch;
    newBatch.quantity = quantity;
    newBatch.expiryDate = expiryDate;
    batches.push_back(newBatch);
    // Simple bubble sort so the batch expiring first stays at index 0
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
// Reduces stock from the earliest-expiring batch first (FEFO rule)
bool Product::reduceStock(int quantity) {
    // Validate quantity against available stock
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
    // Keep only active batches with quantity > 0
    std::vector<Batch> activeBatches;
    for (int i = 0; i < (int)batches.size(); i++) {
        if (batches[i].quantity > 0) {
            activeBatches.push_back(batches[i]);
        }
    }
    batches = activeBatches;
    return true;
}
