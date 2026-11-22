// ============================================================================
// Module: ShelfManager.cpp (Module 9/11)
// Concept: First-Fit Shelf Allocation and Space Balancing
// Data Structure: std::vector<Shelf>
// ============================================================================
#include "ShelfManager.h"
// Default constructor
ShelfManager::ShelfManager() {
}
// Adds a shelf to the collection
void ShelfManager::addShelf(const Shelf& shelf) {
    shelves.push_back(shelf);
}
// Clears all shelves
void ShelfManager::clear() {
    shelves.clear();
}
// Returns reference to shelves vector
std::vector<Shelf>& ShelfManager::getShelves() {
    return shelves;
}
// Returns const reference to shelves vector
const std::vector<Shelf>& ShelfManager::getShelves() const {
    return shelves;
}
// Finds pointer to shelf with matching ID
Shelf* ShelfManager::findShelf(int shelfId) {
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].getShelfId() == shelfId) {
            return &shelves[i];
        }
    }
    return nullptr;
}
// First-Fit algorithm: finds first shelf that can accommodate the quantity
int ShelfManager::findShelfForStock(int quantity) {
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].canFit(quantity)) {
            return shelves[i].getShelfId();
        }
    }
    return -1; // No shelf has sufficient space
}
// Assigns product stock to a designated shelf
bool ShelfManager::assignProductToShelf(int shelfId, int quantity) {
    Shelf* s = findShelf(shelfId);
    if (s == nullptr) {
        return false;
    }
    return s->addUsed(quantity);
}
// Generates utilization and space reports for all shelves
std::vector<ShelfReport> ShelfManager::generateShelfReports() const {
    std::vector<ShelfReport> reports;
    for (int i = 0; i < (int)shelves.size(); i++) {
        ShelfReport rep;
        rep.shelfId = shelves[i].getShelfId();
        rep.name = shelves[i].getName();
        rep.capacity = shelves[i].getCapacity();
        rep.usedSpace = shelves[i].getUsedSpace();
        rep.freeSpace = shelves[i].freeSpace();
        if (rep.capacity > 0) {
            rep.utilizationPercent = ((double)rep.usedSpace / (double)rep.capacity) * 100.0;
        } else {
            rep.utilizationPercent = 0.0;
        }
        reports.push_back(rep);
    }
    return reports;
}
// Suggests moving a product off a crowded shelf (80% or more full).
// The target must be a shelf with the SAME NAME (Dairy -> Dairy) and have at least 25 free space.
// If no such shelf exists, the suggestion tells the owner to add a new shelf (toShelfId = -1).
std::vector<ShelfSuggestion> ShelfManager::suggestRearrangements() const {
    std::vector<ShelfSuggestion> suggestions;
    for (int i = 0; i < (int)shelves.size(); i++) {
        int cap = shelves[i].getCapacity();
        int used = shelves[i].getUsedSpace();
        double util = (cap > 0) ? (((double)used / (double)cap) * 100.0) : 0.0;
        if (util >= 80.0) {
            bool foundTarget = false;
            for (int j = 0; j < (int)shelves.size(); j++) {
                bool sameName = (shelves[j].getName() == shelves[i].getName());
                if (i != j && sameName && shelves[j].freeSpace() >= 25) {
                    ShelfSuggestion sug;
                    sug.fromShelfId = shelves[i].getShelfId();
                    sug.toShelfId = shelves[j].getShelfId();
                    sug.message = "Shelf " + std::to_string(shelves[i].getShelfId()) + " (" + shelves[i].getName() +
                                  ") is crowded (" + std::to_string((int)util) + "% full). Move a product to Shelf " +
                                  std::to_string(shelves[j].getShelfId()) + " (" + shelves[j].getName() +
                                  "), which has " + std::to_string(shelves[j].freeSpace()) + " space free.";
                    suggestions.push_back(sug);
                    foundTarget = true;
                    break;
                }
            }
            if (!foundTarget) {
                ShelfSuggestion sug;
                sug.fromShelfId = shelves[i].getShelfId();
                sug.toShelfId = -1;
                sug.message = "Shelf " + std::to_string(shelves[i].getShelfId()) + " (" + shelves[i].getName() +
                              ") is crowded (" + std::to_string((int)util) + "% full) and no other " +
                              shelves[i].getName() + " shelf has enough room. Add a new " +
                              shelves[i].getName() + " shelf.";
                suggestions.push_back(sug);
            }
        }
    }
    return suggestions;
}
