

#include "ShelfManager.h"

ShelfManager::ShelfManager() {
}

void ShelfManager::addShelf(const Shelf& shelf) {
    shelves.push_back(shelf);
}

void ShelfManager::clear() {
    shelves.clear();
}

std::vector<Shelf>& ShelfManager::getShelves() {
    return shelves;
}

const std::vector<Shelf>& ShelfManager::getShelves() const {
    return shelves;
}

Shelf* ShelfManager::findShelf(int shelfId) {
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].getShelfId() == shelfId) {
            return &shelves[i];
        }
    }
    return nullptr;
}

int ShelfManager::findShelfForStock(int quantity) {
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].canFit(quantity)) {
            return shelves[i].getShelfId();
        }
    }
    return -1; 
}

bool ShelfManager::assignProductToShelf(int shelfId, int quantity) {
    Shelf* s = findShelf(shelfId);
    if (s == nullptr) {
        return false;
    }
    return s->addUsed(quantity);
}

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
