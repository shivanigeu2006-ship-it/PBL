// ============================================================================
// File: main.cpp
// SmartStock: A Retailer-Focused System for Demand-Aware Inventory and FEFO Expiry Control
// Console Interface for shop management and analysis
// ============================================================================
#include "SmartShop.h"
// Displays main menu choices
void displayMenu() {
    std::cout << "\n=======================================================\n";
    std::cout << "           SMARTSTOCK INVENTORY CONTROL SYSTEM         \n";
    std::cout << "=======================================================\n";
    std::cout << " 1. Run Complete 7-Step Workflow\n";
    std::cout << " 2. View Product Catalog & Batches\n";
    std::cout << " 3. Add New Product\n";
    std::cout << " 4. Record a Sale (FEFO Stock Deduction)\n";
    std::cout << " 5. View Smart Alerts & Recommendations\n";
    std::cout << " 6. View Demand Analysis (High/Medium/Low)\n";
    std::cout << " 7. View FEFO Expiry Status (Priority Queue)\n";
    std::cout << " 8. View Shelf Space & Rearrangement Tips\n";
    std::cout << " 9. View Reorder Analysis (ROP & Quantities)\n";
    std::cout << "10. View Profit & Margin Report\n";
    std::cout << "11. Save Data to Files\n";
    std::cout << "12. Add New Shelf\n";
    std::cout << "13. Receive Stock (Reorder Delivery)\n";
    std::cout << "14. Move a Product to Another Shelf\n";
    std::cout << "15. Apply a Shelf Rearrangement Tip\n";
    std::cout << "16. Exit\n";
    std::cout << "-------------------------------------------------------\n";
    std::cout << "Enter your choice (1-16): ";
}
int main() {
    SmartShop shop;
    // Load data or initialize sample data on launch
    shop.loadData();
    shop.generateAlerts();
    int choice = 0;
    while (choice != 16) {
        displayMenu();
        if (!(std::cin >> choice)) {
            std::cin.clear();
            std::string ignoreStr;
            std::cin >> ignoreStr;
            std::cout << "Invalid input. Please enter a number.\n";
            continue;
        }
        if (choice == 1) {
            shop.runWorkflow();
        } else if (choice == 2) {
            std::vector<ProductReport> prods = shop.getProductList();
            std::cout << "\n--- PRODUCT CATALOG (" << prods.size() << " products) ---\n";
            for (int i = 0; i < (int)prods.size(); i++) {
                std::cout << "SKU: " << prods[i].sku << " | Name: " << prods[i].name
                          << " | CP: Rs." << prods[i].costPrice << " | SP: Rs." << prods[i].sellingPrice
                          << " | Shelf: " << prods[i].shelfId << " (" << prods[i].shelfName << ")" << " | Space/Unit: " << prods[i].spacePerUnit << " | Total Stock: " << prods[i].totalStock << "\n";
                for (int b = 0; b < (int)prods[i].batches.size(); b++) {
                    std::cout << "   -> Batch " << (b + 1) << ": Qty = " << prods[i].batches[b].quantity
                              << ", Expiry = " << formatDate(prods[i].batches[b].expiryDate) << "\n";
                }
            }
        } else if (choice == 3) {
            std::string sku;
            std::string name;
            double cp = 0.0;
            double sp = 0.0;
            int shelfId = 1;
            int spaceUnit = 1;
            int qty = 0;
            int exp = 0;
            std::cout << "Enter SKU (e.g. JUIC707): ";
            std::cin >> sku;
            std::cout << "Enter Product Name: ";
            std::cin.ignore();
            std::getline(std::cin, name);
            std::cout << "Enter Cost Price (Rs.): ";
            std::cin >> cp;
            std::cout << "Enter Selling Price (Rs.): ";
            std::cin >> sp;
            std::vector<ShelfReport> shelfList = shop.getShelfReport();
            std::cout << "Available shelves:\n";
            for (int i = 0; i < (int)shelfList.size(); i++) {
                std::cout << "  Shelf " << shelfList[i].shelfId << " - " << shelfList[i].name
                          << " (free space: " << shelfList[i].freeSpace << ")\n";
            }
            std::cout << "Enter Shelf ID: ";
            std::cin >> shelfId;
            std::cout << "Enter Space per Unit (1 = small packet, 2 = bigger, etc.): ";
            std::cin >> spaceUnit;
            std::cout << "Enter Initial Batch Quantity: ";
            std::cin >> qty;
            std::cout << "Enter Expiry Date (YYYY-MM-DD, e.g. 2026-12-15): ";
            std::string expText;
            std::cin >> expText;
            exp = parseDate(expText);
            if (exp == 0) {
                std::cout << "Invalid date! Please use the format YYYY-MM-DD.\n";
            } else {
                shop.addProduct(sku, name, cp, sp, shelfId, spaceUnit, qty, exp);
            }
        } else if (choice == 4) {
            std::string sku;
            int units = 0;
            int day = 1;
            std::cout << "Enter SKU: ";
            std::cin >> sku;
            std::cout << "Enter Units Sold: ";
            std::cin >> units;
            std::cout << "Enter Day Number: ";
            std::cin >> day;
            shop.recordSale(sku, units, day);
        } else if (choice == 5) {
            std::vector<SmartAlert> alerts = shop.getAlerts();
            std::cout << "\n--- SMART ALERTS & RECOMMENDATIONS (" << alerts.size() << ") ---\n";
            if (alerts.empty()) {
                std::cout << "No current alerts. All inventory levels and expiries are healthy!\n";
            }
            for (int i = 0; i < (int)alerts.size(); i++) {
                std::cout << "[" << alerts[i].severity << "] (" << alerts[i].type << ") "
                          << alerts[i].message << "\n";
            }
        } else if (choice == 6) {
            std::vector<DemandItem> demand = shop.getDemandReport();
            std::cout << "\n--- DEMAND ANALYSIS REPORT (Descending) ---\n";
            for (int i = 0; i < (int)demand.size(); i++) {
                std::cout << "SKU: " << demand[i].sku << " (" << demand[i].productName << ")"
                          << " | Sold: " << demand[i].totalUnitsSold
                          << " | Avg Daily Demand: " << demand[i].averageDailyDemand
                          << " | Class: " << demand[i].demandCategory << "\n";
            }
        } else if (choice == 7) {
            std::vector<ExpiryItem> expReport = shop.getExpiryReport();
            std::cout << "\n--- FEFO EXPIRY REPORT (Priority Queue Min-Heap) ---\n";
            for (int i = 0; i < (int)expReport.size(); i++) {
                std::cout << "SKU: " << expReport[i].sku << " (" << expReport[i].productName << ")"
                          << " | Qty: " << expReport[i].quantity
                          << " | Expiry: " << formatDate(expReport[i].expiryDate)
                          << " | Days Left: " << expReport[i].daysLeft
                          << " | Status: " << expReport[i].status << "\n";
            }
        } else if (choice == 8) {
            std::vector<ShelfReport> shelves = shop.getShelfReport();
            std::cout << "\n--- SHELF SPACE UTILIZATION ---\n";
            for (int i = 0; i < (int)shelves.size(); i++) {
                std::cout << "Shelf " << shelves[i].shelfId << " (" << shelves[i].name << ")"
                          << " | Cap: " << shelves[i].capacity
                          << " | Used: " << shelves[i].usedSpace
                          << " | Free: " << shelves[i].freeSpace
                          << " | Util: " << shelves[i].utilizationPercent << "%\n";
            }
            std::vector<ShelfSuggestion> tips = shop.getShelfRearrangements();
            std::cout << "\n--- REARRANGEMENT TIPS ---\n";
            if (tips.empty()) {
                std::cout << "All shelves have balanced capacity.\n";
            }
            for (int i = 0; i < (int)tips.size(); i++) {
                std::cout << "-> Tip " << (i + 1) << ": " << tips[i].message << "\n";
            }
        } else if (choice == 9) {
            std::vector<ReorderItem> reorders = shop.getReorderReport();
            std::cout << "\n--- REORDER RECOMMENDATIONS ---\n";
            for (int i = 0; i < (int)reorders.size(); i++) {
                std::cout << "SKU: " << reorders[i].sku << " (" << reorders[i].productName << ")"
                          << " | Stock: " << reorders[i].currentStock
                          << " | ROP: " << reorders[i].reorderPoint
                          << " | Target: " << reorders[i].targetStock
                          << " | Needs Reorder: " << (reorders[i].needsReorder ? "YES" : "NO")
                          << " | Suggested Order: " << reorders[i].suggestedQuantity
                          << (reorders[i].limitedByShelf ? " (limited by shelf space)" : "") << "\n";
            }
        } else if (choice == 10) {
            ProfitSummary pSummary = shop.getProfitReport();
            std::cout << "\n--- PROFIT & MARGIN REPORT ---\n";
            for (int i = 0; i < (int)pSummary.items.size(); i++) {
                std::cout << "SKU: " << pSummary.items[i].sku << " (" << pSummary.items[i].productName << ")"
                          << " | Sold: " << pSummary.items[i].unitsSold
                          << " | Unit Profit: Rs." << pSummary.items[i].profitPerUnit
                          << " | Total Profit: Rs." << pSummary.items[i].totalProfit
                          << " | Margin: " << pSummary.items[i].profitMarginPercent << "%\n";
            }
            std::cout << "-------------------------------------------------------\n";
            std::cout << "Grand Total Profit: Rs." << pSummary.grandTotalProfit
                      << " | Average Margin: " << pSummary.averageMargin << "%\n";
        } else if (choice == 11) {
            shop.saveData();
        } else if (choice == 12) {
            std::string shelfName;
            int cap = 0;
            std::cout << "Enter shelf name/type (e.g. Dairy, Frozen, General): ";
            std::cin.ignore();
            std::getline(std::cin, shelfName);
            std::cout << "Enter capacity of the new shelf (in space units): ";
            std::cin >> cap;
            int newId = shop.addNewShelf(shelfName, cap);
            if (newId > 0) {
                shop.saveData();
            }
        } else if (choice == 13) {
            std::string sku;
            int qty = 0;
            std::string expText;
            std::cout << "Enter SKU to restock: ";
            std::cin >> sku;
            std::vector<ReorderItem> reorders = shop.getReorderReport();
            for (int i = 0; i < (int)reorders.size(); i++) {
                if (reorders[i].sku == sku) {
                    std::cout << "Current stock: " << reorders[i].currentStock
                              << " | Suggested order quantity: " << reorders[i].suggestedQuantity << "\n";
                }
            }
            std::cout << "Enter Quantity Received: ";
            std::cin >> qty;
            std::cout << "Enter Expiry Date of this batch (YYYY-MM-DD): ";
            std::cin >> expText;
            int exp = parseDate(expText);
            if (exp == 0) {
                std::cout << "Invalid date! Please use the format YYYY-MM-DD.\n";
            } else if (shop.addBatchToProduct(sku, qty, exp)) {
                shop.generateAlerts();
                shop.saveData();
            }
        } else if (choice == 14) {
            std::string sku;
            int toShelf = 0;
            std::cout << "Enter SKU of the product to move: ";
            std::cin >> sku;
            std::cout << "Enter the ID of the shelf to move it to: ";
            std::cin >> toShelf;
            if (shop.moveProductToShelf(sku, toShelf)) {
                shop.saveData();
            }
        } else if (choice == 15) {
            std::vector<ShelfSuggestion> tips = shop.getShelfRearrangements();
            if (tips.empty()) {
                std::cout << "No rearrangement tips right now. All shelves have enough room.\n";
            } else {
                for (int i = 0; i < (int)tips.size(); i++) {
                    std::cout << "Tip " << (i + 1) << ": " << tips[i].message << "\n";
                }
                int tipNumber = 0;
                std::cout << "Enter the tip number to apply (0 to cancel): ";
                std::cin >> tipNumber;
                if (tipNumber >= 1 && tipNumber <= (int)tips.size()) {
                    if (tips[tipNumber - 1].toShelfId < 0) {
                        std::cout << "This tip needs a new shelf. Use option 12 to add one, then move a product (option 14).\n";
                    } else if (shop.applyRearrangement(tips[tipNumber - 1].fromShelfId, tips[tipNumber - 1].toShelfId)) {
                        shop.saveData();
                    }
                }
            }
        } else if (choice == 16) {
            std::cout << "Saving data before exit...\n";
            shop.saveData();
            std::cout << "Thank you for using SmartStock!\n";
        } else {
            std::cout << "Invalid choice! Please select 1 through 16.\n";
        }
    }
    return 0;
}
