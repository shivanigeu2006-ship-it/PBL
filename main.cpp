// ============================================================
// main.cpp  -  Interactive menu for the 8 completed SmartStock modules
// The user types all the data. Nothing is added automatically.
// There are no header files, so main.cpp includes the module files.
// ONLY main.cpp is compiled (it pulls in all the other .cpp files).
// ============================================================
#include <iostream>
#include <string>
#include <vector>
#include <sstream>
#include <cstdlib>
#include "Product.cpp"
#include "Shelf.cpp"
#include "Inventory.cpp"
#include "SalesRecord.cpp"
#include "Supplier.cpp"
#include "DemandAnalyzer.cpp"
#include "ExpiryManager.cpp"
#include "ReorderManager.cpp"
using namespace std;

// ---------------- Input helper functions ----------------

// Shows a message and reads one full line from the user
string readLine(const string& prompt) {
    string line;
    cout << prompt;
    if (!getline(cin, line)) {
        cout << "\nInput ended. Exiting program.\n";
        exit(0);
    }
    return line;
}

// Reads text that cannot be empty (spaces at both ends are removed)
string readText(const string& prompt) {
    while (true) {
        string line = readLine(prompt);
        size_t start = line.find_first_not_of(" \t");
        size_t end = line.find_last_not_of(" \t");
        if (start != string::npos) {
            return line.substr(start, end - start + 1);
        }
        cout << "Input cannot be empty. Please try again.\n";
    }
}

// Reads a whole number that is at least minValue
int readInt(const string& prompt, int minValue) {
    while (true) {
        string line = readLine(prompt);
        istringstream in(line);
        int value;
        char extra;
        if (in >> value && !(in >> extra)) {
            if (value >= minValue) return value;
            cout << "Value must be at least " << minValue << ". Please try again.\n";
        } else {
            cout << "Invalid input. Please enter a whole number.\n";
        }
    }
}

// Reads a number (decimals allowed) that is at least minValue
double readDouble(const string& prompt, double minValue) {
    while (true) {
        string line = readLine(prompt);
        istringstream in(line);
        double value;
        char extra;
        if (in >> value && !(in >> extra)) {
            if (value >= minValue) return value;
            cout << "Value must be at least " << minValue << ". Please try again.\n";
        } else {
            cout << "Invalid input. Please enter a number.\n";
        }
    }
}

// Reads day, month and year until a valid date is entered
Date readDate(const string& label) {
    while (true) {
        cout << label << "\n";
        int d = readInt("  Day   : ", 1);
        int m = readInt("  Month : ", 1);
        int y = readInt("  Year  : ", 1);
        Date date(d, m, y);
        if (date.isValid()) return date;
        cout << "Invalid date. Please try again.\n";
    }
}

// ---------------- Menu actions ----------------

void addProduct(Inventory& inventory) {
    cout << "\n--- Add Product ---\n";
    string sku = readText("SKU (unique code): ");
    if (inventory.productExists(sku)) {
        cout << "Error: SKU " << sku << " already exists.\n";
        return;
    }
    string name = readText("Product name: ");
    double cost = readDouble("Cost price: ", 0);
    double price = readDouble("Selling price: ", 0);

    Product product(sku, name, cost, price);
    if (inventory.addProduct(product)) {
        cout << "Product added. Use option 2 to add stock to it.\n";
    }
}

void addStock(Inventory& inventory) {
    cout << "\n--- Add Stock (new batch) ---\n";
    string sku = readText("SKU: ");
    if (!inventory.productExists(sku)) {
        cout << "Error: SKU " << sku << " not found. Add the product first.\n";
        return;
    }
    int quantity = readInt("Quantity (units): ", 1);
    Date expiry = readDate("Expiry date of this batch:");
    if (inventory.addStock(sku, quantity, expiry)) {
        cout << "Stock added. Total stock of " << sku << ": " << inventory.getStock(sku) << "\n";
    }
}

void recordSale(Inventory& inventory, SalesRecord& sales) {
    cout << "\n--- Record Sale ---\n";
    string sku = readText("SKU: ");
    if (!inventory.productExists(sku)) {
        cout << "Error: SKU " << sku << " not found.\n";
        return;
    }
    int units = readInt("Units sold: ", 1);
    Date date = readDate("Sale date:");
    if (sales.recordSale(inventory, sku, units, date)) {
        cout << "Sale recorded. Remaining stock of " << sku << ": "
             << inventory.getStock(sku) << "\n";
    }
}

// ----- Shelf helpers -----
Shelf* findShelf(vector<Shelf>& shelves, const string& id) {
    for (size_t i = 0; i < shelves.size(); i++) {
        if (shelves[i].getShelfId() == id) return &shelves[i];
    }
    return nullptr;
}

void shelfMenu(vector<Shelf>& shelves) {
    while (true) {
        cout << "\n--- Shelf Menu ---\n";
        cout << "1. Create shelf\n";
        cout << "2. Add units to shelf\n";
        cout << "3. Remove units from shelf\n";
        cout << "4. Show all shelves\n";
        cout << "0. Back to main menu\n";
        int choice = readInt("Enter choice: ", 0);

        if (choice == 0) return;

        if (choice == 1) {
            string id = readText("Shelf ID: ");
            if (findShelf(shelves, id) != nullptr) {
                cout << "Error: shelf " << id << " already exists.\n";
            } else {
                int capacity = readInt("Capacity (units): ", 1);
                shelves.push_back(Shelf(id, capacity));
                cout << "Shelf created.\n";
            }
        } else if (choice == 2 || choice == 3) {
            string id = readText("Shelf ID: ");
            Shelf* shelf = findShelf(shelves, id);
            if (shelf == nullptr) {
                cout << "Error: shelf " << id << " not found.\n";
            } else {
                int units = readInt("Units: ", 1);
                bool ok = (choice == 2) ? shelf->addUnits(units) : shelf->removeUnits(units);
                if (ok) {
                    cout << "Done. ";
                    shelf->display();
                }
            }
        } else if (choice == 4) {
            if (shelves.empty()) {
                cout << "No shelves created yet.\n";
            }
            for (size_t i = 0; i < shelves.size(); i++) {
                shelves[i].display();
            }
        } else {
            cout << "Invalid choice. Please try again.\n";
        }
    }
}

// ----- Supplier helpers -----
Supplier* findSupplier(vector<Supplier>& suppliers, const string& id) {
    for (size_t i = 0; i < suppliers.size(); i++) {
        if (suppliers[i].getSupplierId() == id) return &suppliers[i];
    }
    return nullptr;
}

void addSupplier(vector<Supplier>& suppliers) {
    cout << "\n--- Add Supplier ---\n";
    string id = readText("Supplier ID: ");
    if (findSupplier(suppliers, id) != nullptr) {
        cout << "Error: supplier " << id << " already exists.\n";
        return;
    }
    string name = readText("Supplier name: ");
    string contact = readText("Contact (phone/email): ");
    int leadTime = readInt("Lead time (days): ", 0);
    int safetyStock = readInt("Safety stock (units): ", 0);
    suppliers.push_back(Supplier(id, name, contact, leadTime, safetyStock));
    cout << "Supplier added.\n";
}

void showSuppliers(const vector<Supplier>& suppliers) {
    cout << "\n--- Suppliers ---\n";
    if (suppliers.empty()) {
        cout << "No suppliers added yet.\n";
    }
    for (size_t i = 0; i < suppliers.size(); i++) {
        suppliers[i].display();
    }
}

void demandAnalysis(const Inventory& inventory, const SalesRecord& sales, DemandAnalyzer& demand) {
    cout << "\n--- Demand Analysis ---\n";
    int days = readInt("Number of days the sales records cover: ", 1);
    if (demand.analyze(inventory, sales, days)) {
        demand.displayReport();
    }
}

void expiryReport(const Inventory& inventory, ExpiryManager& expiry) {
    cout << "\n--- Expiry Report (FEFO) ---\n";
    Date today = readDate("Today's date:");
    int soonDays = readInt("Treat as 'expiring soon' if days left is up to: ", 0);
    expiry.setSoonDays(soonDays);
    expiry.loadBatches(inventory);

    cout << "\nAll batches in FEFO order (earliest expiry first):\n";
    expiry.displayFEFOOrder(today);

    ExpiryEntry next;
    if (expiry.getNextBatchToSell(today, next)) {
        cout << "\nNext batch to sell (FEFO): " << next.sku << ", " << next.quantity
             << " units, expiry " << next.expiry.toString() << "\n";
    }

    cout << "\n";
    expiry.displayAlerts(today);
}

void reorderAnalysis(const Inventory& inventory, const SalesRecord& sales,
                     vector<Supplier>& suppliers, DemandAnalyzer& demand,
                     const ReorderManager& reorder) {
    cout << "\n--- Reorder Analysis ---\n";
    string sku = readText("SKU: ");
    const Product* product = inventory.findProduct(sku);
    if (product == nullptr) {
        cout << "Error: SKU " << sku << " not found.\n";
        return;
    }
    if (suppliers.empty()) {
        cout << "Error: no supplier added yet. Use option 7 first.\n";
        return;
    }
    string supplierId = readText("Supplier ID: ");
    Supplier* supplier = findSupplier(suppliers, supplierId);
    if (supplier == nullptr) {
        cout << "Error: supplier " << supplierId << " not found.\n";
        return;
    }

    int days = readInt("Number of days the sales records cover: ", 1);
    if (!demand.analyze(inventory, sales, days)) {
        return;
    }
    int targetStock = readInt("Target stock (units): ", 0);

    double avgDemand = demand.getAverageDemand(sku);
    cout << "\nAverage daily demand of " << sku << ": " << avgDemand << "\n";
    cout << "Lead time: " << supplier->getLeadTimeDays() << " days, safety stock: "
         << supplier->getSafetyStock() << "\n";
    ReorderInfo info = reorder.analyzeProduct(*product, avgDemand, *supplier, targetStock);
    reorder.displayReorderInfo(info);
}

void showMenu() {
    cout << "\n==================== SmartStock ====================\n";
    cout << " 1. Add product\n";
    cout << " 2. Add stock (new batch with expiry date)\n";
    cout << " 3. Show all products\n";
    cout << " 4. Record a sale\n";
    cout << " 5. Show sales records\n";
    cout << " 6. Shelf menu\n";
    cout << " 7. Add supplier\n";
    cout << " 8. Show suppliers\n";
    cout << " 9. Demand analysis\n";
    cout << "10. Expiry report (FEFO)\n";
    cout << "11. Reorder analysis\n";
    cout << " 0. Exit\n";
    cout << "====================================================\n";
}

int main() {
    Inventory inventory;
    SalesRecord sales;
    vector<Shelf> shelves;
    vector<Supplier> suppliers;
    DemandAnalyzer demand;
    ExpiryManager expiry;
    ReorderManager reorder;

    while (true) {
        showMenu();
        int choice = readInt("Enter choice: ", 0);

        if (choice == 0) {
            cout << "Goodbye!\n";
            break;
        } else if (choice == 1) {
            addProduct(inventory);
        } else if (choice == 2) {
            addStock(inventory);
        } else if (choice == 3) {
            cout << "\n--- All Products ---\n";
            inventory.displayAll();
        } else if (choice == 4) {
            recordSale(inventory, sales);
        } else if (choice == 5) {
            cout << "\n--- Sales Records ---\n";
            sales.displayAll();
        } else if (choice == 6) {
            shelfMenu(shelves);
        } else if (choice == 7) {
            addSupplier(suppliers);
        } else if (choice == 8) {
            showSuppliers(suppliers);
        } else if (choice == 9) {
            demandAnalysis(inventory, sales, demand);
        } else if (choice == 10) {
            expiryReport(inventory, expiry);
        } else if (choice == 11) {
            reorderAnalysis(inventory, sales, suppliers, demand, reorder);
        } else {
            cout << "Invalid choice. Please enter a number from the menu.\n";
        }
    }
    return 0;
}