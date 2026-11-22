#ifndef SHELFMANAGER_H
#define SHELFMANAGER_H
#include <iostream>
#include <iomanip>
#include <string>
#include <vector>
#include "Shelf.h"

struct ShelfSuggestion {
    int fromShelfId;
    int toShelfId;
    std::string message;
};

class ShelfManager {
private:
    std::vector<Shelf> shelves;
public:
    ShelfManager();
    void addShelf(const Shelf& shelf);
    int findShelfForStock(int quantity); 
    bool assignProductToShelf(int shelfId, int quantity);
    std::vector<ShelfReport> generateShelfReports() const;
    std::vector<ShelfSuggestion> suggestRearrangements() const;
    std::vector<Shelf>& getShelves();
    const std::vector<Shelf>& getShelves() const;
    Shelf* findShelf(int shelfId);
    void clear();
};
#endif
