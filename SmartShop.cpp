// ============================================================================
// Module: SmartShop.cpp (Module 11/11)
// Concept: Composition, System Integration, 7-Step Workflow, File Handling
// ============================================================================
#include "SmartShop.h"
// Default constructor initializes default date and simulation periods
SmartShop::SmartShop() {
    simulatedCurrentDate = 20261122; // Nov 22, 2026
    analysisPeriodDays = 7;          // 7-day demand analysis window
}
// Returns current simulated date
int SmartShop::getCurrentDate() const {
    return simulatedCurrentDate;
}
// Sets current simulated date
void SmartShop::setCurrentDate(int date) {
    if (date > 0) {
        simulatedCurrentDate = date;
    }
}
// Returns analysis period in days
int SmartShop::getAnalysisDays() const {
    return analysisPeriodDays;
}
// Sets analysis period in days
void SmartShop::setAnalysisDays(int days) {
    if (days >= 1) {
        analysisPeriodDays = days;
    }
}
// Adds a shelf directly
void SmartShop::addShelf(int shelfId, const std::string& name, int capacity) {
    Shelf s(shelfId, name, capacity);
    shelfManager.addShelf(s);
}
// Adds a new shelf with the next free ID; returns the ID, or -1 if the name or capacity is invalid
int SmartShop::addNewShelf(const std::string& name, int capacity) {
    if (name.empty() || capacity <= 0) {
        std::cout << "Invalid shelf name or capacity!\n";
        return -1;
    }
    int maxId = 0;
    const std::vector<Shelf>& shelves = shelfManager.getShelves();
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].getShelfId() > maxId) {
            maxId = shelves[i].getShelfId();
        }
    }
    int newId = maxId + 1;
    shelfManager.addShelf(Shelf(newId, name, capacity));
    std::cout << "Shelf " << newId << " (" << name << ") added with capacity " << capacity << "!\n";
    return newId;
}
// Returns the name of a shelf, or "Unknown" if the ID does not exist
std::string SmartShop::getShelfName(int shelfId) const {
    const std::vector<Shelf>& shelves = shelfManager.getShelves();
    for (int i = 0; i < (int)shelves.size(); i++) {
        if (shelves[i].getShelfId() == shelfId) {
            return shelves[i].getName();
        }
    }
    return "Unknown";
}
// Stores a result message and also prints it
void SmartShop::setMessage(const std::string& message) {
    lastMessage = message;
    std::cout << message << "\n";
}
// Returns the message from the last move action
std::string SmartShop::getLastMessage() const {
    return lastMessage;
}
// Moves a whole product (all its batches) to another shelf.
// Rule: the new shelf must have the same name as the current shelf, and must have enough free space.
bool SmartShop::moveProductToShelf(const std::string& sku, int newShelfId) {
    Product* p = inventory.findProduct(sku);
    if (p == nullptr) {
        setMessage("Product with SKU " + sku + " not found!");
        return false;
    }
    Shelf* fromShelf = shelfManager.findShelf(p->getShelfId());
    Shelf* toShelf = shelfManager.findShelf(newShelfId);
    if (fromShelf == nullptr || toShelf == nullptr) {
        setMessage("Shelf " + std::to_string(newShelfId) + " not found!");
        return false;
    }
    if (fromShelf->getShelfId() == toShelf->getShelfId()) {
        setMessage("The product is already on Shelf " + std::to_string(newShelfId) + "!");
        return false;
    }
    if (fromShelf->getName() != toShelf->getName()) {
        setMessage("Cannot move: " + p->getName() + " is on a " + fromShelf->getName() +
                   " shelf, but Shelf " + std::to_string(newShelfId) + " is a " + toShelf->getName() +
                   " shelf. Products can only move to a shelf with the same name.");
        return false;
    }
    int spaceNeeded = p->getSpaceUsed();
    if (!toShelf->canFit(spaceNeeded)) {
        setMessage("Shelf " + std::to_string(newShelfId) + " only has " + std::to_string(toShelf->freeSpace()) +
                   " free space, but " + p->getName() + " needs " + std::to_string(spaceNeeded) + ".");
        return false;
    }
    if (spaceNeeded > 0) {
        toShelf->addUsed(spaceNeeded);
        fromShelf->removeUsed(spaceNeeded);
    }
    p->setShelfId(newShelfId);
    setMessage("Moved " + p->getName() + " (" + sku + ") from Shelf " + std::to_string(fromShelf->getShelfId()) +
               " to Shelf " + std::to_string(newShelfId) + " (" + toShelf->getName() + ").");
    return true;
}
// Carries out a rearrangement tip: moves the biggest product from the crowded shelf
// that still fits on the target shelf.
bool SmartShop::applyRearrangement(int fromShelfId, int toShelfId) {
    Shelf* toShelf = shelfManager.findShelf(toShelfId);
    if (toShelf == nullptr) {
        setMessage("Target shelf not found. Add a new shelf first.");
        return false;
    }
    std::string bestSku = "";
    int bestSpace = 0;
    for (int i = 0; i < inventory.getProductCount(); i++) {
        Product* p = inventory.getProductByIndex(i);
        if (p->getShelfId() == fromShelfId) {
            int space = p->getSpaceUsed();
            if (space > bestSpace && toShelf->canFit(space)) {
                bestSpace = space;
                bestSku = p->getSku();
            }
        }
    }
    if (bestSku.empty()) {
        setMessage("No product on Shelf " + std::to_string(fromShelfId) + " fits on Shelf " +
                   std::to_string(toShelfId) + ". Try adding a bigger shelf.");
        return false;
    }
    return moveProductToShelf(bestSku, toShelfId);
}
// Adds a supplier directly
void SmartShop::addSupplier(const Supplier& supplier) {
    suppliers.push_back(supplier);
}
// Pre-loads sample retail data if files do not exist
void SmartShop::preloadSampleData() {
    // 1. Setup Shelves (each shelf has a name; a product can only sit on a shelf of its own kind)
    shelfManager.clear();
    shelfManager.addShelf(Shelf(1, "Dairy", 120));
    shelfManager.addShelf(Shelf(2, "Frozen", 80));
    shelfManager.addShelf(Shelf(3, "General", 300));
    shelfManager.addShelf(Shelf(4, "Dairy", 60)); // an extra dairy shelf
    // 2. Setup Products with batches (Product(sku, name, costPrice, sellingPrice, shelfId, spacePerUnit))
    inventory.clear();
    Product p1("MILK101", "Organic Whole Milk", 48.00, 60.00, 1, 2);
    p1.addBatch(15, 20261124); // Expiring soon (2 days left)
    p1.addBatch(25, 20261205); // Good
    inventory.addProduct(p1);
    Product p2("YOGU202", "Greek Yogurt Cup", 22.00, 30.00, 1, 1);
    p2.addBatch(8, 20261120);  // Already Expired
    p2.addBatch(20, 20261201); // Good
    inventory.addProduct(p2);
    Product p3("BRED303", "Whole Wheat Bread", 35.00, 45.00, 3, 2);
    p3.addBatch(10, 20261125); // Expiring soon (3 days left)
    p3.addBatch(30, 20261210); // Good
    inventory.addProduct(p3);
    Product p4("EGGS404", "Farm Fresh Eggs 12pk", 72.00, 95.00, 3, 2);
    p4.addBatch(12, 20261128); // Expiring soon (6 days left)
    p4.addBatch(28, 20261215); // Good
    inventory.addProduct(p4);
    Product p5("APPL505", "Crisp Red Apples", 90.00, 130.00, 3, 1);
    p5.addBatch(30, 20261202);
    p5.addBatch(45, 20261220);
    inventory.addProduct(p5);
    Product p6("CHEZ606", "Cheddar Cheese Block", 110.00, 150.00, 4, 1);
    p6.addBatch(5, 20261122);  // Expiring today
    p6.addBatch(5, 20261225);
    inventory.addProduct(p6);
    Product p7("ICRM707", "Vanilla Ice Cream Tub", 120.00, 180.00, 2, 2);
    p7.addBatch(10, 20261230);
    p7.addBatch(15, 20270215);
    inventory.addProduct(p7);
    Product p8("PEAS808", "Frozen Green Peas", 55.00, 75.00, 2, 1);
    p8.addBatch(20, 20270120);
    inventory.addProduct(p8);
    // 3. Setup Suppliers
    suppliers.clear();
    suppliers.push_back(Supplier("DairyFresh Co", "MILK101", 2, 15));
    suppliers.push_back(Supplier("GreekDelight Ltd", "YOGU202", 3, 10));
    suppliers.push_back(Supplier("GoldenBakery", "BRED303", 2, 12));
    suppliers.push_back(Supplier("SunnyFarms", "EGGS404", 3, 15));
    suppliers.push_back(Supplier("OrchardHarvest", "APPL505", 4, 20));
    suppliers.push_back(Supplier("ValleyCheese", "CHEZ606", 5, 10));
    suppliers.push_back(Supplier("FrostyFoods Ltd", "ICRM707", 4, 10));
    suppliers.push_back(Supplier("FrostyFoods Ltd", "PEAS808", 3, 10));
    // 4. Setup Sales History
    salesRecord.clear();
    SaleEntry s1 = {"MILK101", 10, 1}; salesRecord.addSaleDirect(s1);
    SaleEntry s2 = {"MILK101", 12, 3}; salesRecord.addSaleDirect(s2);
    SaleEntry s3 = {"MILK101", 8, 5};  salesRecord.addSaleDirect(s3);
    SaleEntry s4 = {"YOGU202", 5, 2};  salesRecord.addSaleDirect(s4);
    SaleEntry s5 = {"YOGU202", 6, 4};  salesRecord.addSaleDirect(s5);
    SaleEntry s6 = {"BRED303", 15, 1}; salesRecord.addSaleDirect(s6);
    SaleEntry s7 = {"BRED303", 15, 3}; salesRecord.addSaleDirect(s7);
    SaleEntry s8 = {"BRED303", 10, 6}; salesRecord.addSaleDirect(s8);
    SaleEntry s9 = {"EGGS404", 8, 2};  salesRecord.addSaleDirect(s9);
    SaleEntry s10 = {"EGGS404", 12, 5}; salesRecord.addSaleDirect(s10);
    SaleEntry s11 = {"APPL505", 5, 1};  salesRecord.addSaleDirect(s11);
    SaleEntry s12 = {"APPL505", 5, 4};  salesRecord.addSaleDirect(s12);
    SaleEntry s13 = {"CHEZ606", 15, 3}; salesRecord.addSaleDirect(s13);
    SaleEntry s14 = {"CHEZ606", 10, 6}; salesRecord.addSaleDirect(s14);
    SaleEntry s15 = {"ICRM707", 6, 2};  salesRecord.addSaleDirect(s15);
    SaleEntry s16 = {"ICRM707", 8, 5};  salesRecord.addSaleDirect(s16);
    SaleEntry s17 = {"PEAS808", 10, 3}; salesRecord.addSaleDirect(s17);
    manageInventory();
    std::cout << "Sample retail data preloaded successfully!\n";
}
// Caps each suggested order quantity to what actually fits on the product's shelf
void SmartShop::limitToShelfSpace(std::vector<ReorderItem>& items) const {
    const std::vector<Shelf>& shelves = shelfManager.getShelves();
    for (int i = 0; i < (int)items.size(); i++) {
        if (items[i].suggestedQuantity <= 0) {
            continue;
        }
        const Product* p = inventory.findProduct(items[i].sku);
        if (p == nullptr) {
            continue;
        }
        for (int s = 0; s < (int)shelves.size(); s++) {
            if (shelves[s].getShelfId() == p->getShelfId()) {
                int maxUnits = shelves[s].freeSpace() / p->getSpacePerUnit();
                if (items[i].suggestedQuantity > maxUnits) {
                    items[i].suggestedQuantity = maxUnits;
                    items[i].limitedByShelf = true;
                }
                break;
            }
        }
    }
}
// Step 1: Load Data from text files (starts with sample data if files are missing)
void SmartShop::loadData() {
    std::ifstream pfile("products.txt");
    if (!pfile.is_open()) {
        std::cout << "No saved data found. Pre-loading sample data...\n";
        preloadSampleData();
        saveData();
        return;
    }
    // 1. Load Shelves
    std::ifstream sfile("shelves.txt");
    shelfManager.clear();
    if (sfile.is_open()) {
        int shelfCount = 0;
        sfile >> shelfCount;
        for (int i = 0; i < shelfCount; i++) {
            int id = 0;
            std::string rawShelfName = "";
            int cap = 0;
            int used = 0;
            sfile >> id >> rawShelfName >> cap >> used;
            // Replace underscores with spaces in the shelf name
            std::string cleanShelfName = rawShelfName;
            for (int c = 0; c < (int)cleanShelfName.size(); c++) {
                if (cleanShelfName[c] == '_') {
                    cleanShelfName[c] = ' ';
                }
            }
            Shelf sh(id, cleanShelfName, cap);
            sh.setUsedSpace(used);
            shelfManager.addShelf(sh);
        }
        sfile.close();
    }
    // 2. Load Products and Batches
    inventory.clear();
    int prodCount = 0;
    pfile >> prodCount;
    for (int i = 0; i < prodCount; i++) {
        std::string sku = "";
        std::string rawName = "";
        double cp = 0.0;
        double sp = 0.0;
        int shelfId = 0;
        int spaceUnit = 1;
        int batchCount = 0;
        pfile >> sku >> rawName >> cp >> sp >> shelfId >> spaceUnit >> batchCount;
        // Replace underscores with spaces in product name
        std::string cleanName = rawName;
        for (int c = 0; c < (int)cleanName.size(); c++) {
            if (cleanName[c] == '_') {
                cleanName[c] = ' ';
            }
        }
        Product prod(sku, cleanName, cp, sp, shelfId, spaceUnit);
        for (int b = 0; b < batchCount; b++) {
            int qty = 0;
            int exp = 0;
            pfile >> qty >> exp;
            prod.addBatch(qty, exp);
        }
        inventory.addProduct(prod);
    }
    pfile.close();
    // 3. Load Sales Records
    std::ifstream salefile("sales.txt");
    salesRecord.clear();
    if (salefile.is_open()) {
        int saleCount = 0;
        salefile >> saleCount;
        for (int i = 0; i < saleCount; i++) {
            SaleEntry entry;
            salefile >> entry.sku >> entry.unitsSold >> entry.dayNumber;
            salesRecord.addSaleDirect(entry);
        }
        salefile.close();
    }
    // 4. Load Suppliers
    std::ifstream supfile("suppliers.txt");
    suppliers.clear();
    if (supfile.is_open()) {
        int supCount = 0;
        supfile >> supCount;
        for (int i = 0; i < supCount; i++) {
            std::string sku = "";
            std::string rawSupName = "";
            int leadTime = 0;
            int safetyStock = 0;
            supfile >> sku >> rawSupName >> leadTime >> safetyStock;
            std::string cleanSup = rawSupName;
            for (int c = 0; c < (int)cleanSup.size(); c++) {
                if (cleanSup[c] == '_') {
                    cleanSup[c] = ' ';
                }
            }
            suppliers.push_back(Supplier(cleanSup, sku, leadTime, safetyStock));
        }
        supfile.close();
    }
    manageInventory();
    std::cout << "Data loaded successfully from text files.\n";
}
// Step 2: Manage Inventory - recalculates used shelf space from product stock
void SmartShop::manageInventory() {
    std::vector<Shelf>& shelves = shelfManager.getShelves();
    for (int i = 0; i < (int)shelves.size(); i++) {
        shelves[i].setUsedSpace(0);
    }
    std::vector<Product> products = inventory.getAllProducts();
    for (int p = 0; p < (int)products.size(); p++) {
        int shelfId = products[p].getShelfId();
        int stock = products[p].getSpaceUsed(); // units * space per unit
        for (int s = 0; s < (int)shelves.size(); s++) {
            if (shelves[s].getShelfId() == shelfId) {
                shelves[s].addUsed(stock);
                break;
            }
        }
    }
}
// Step 3: Run Analysis Engine - Demand, Expiry, and Shelf Space in parallel branches
void SmartShop::runAnalysisEngine() {
    // Branch A: Demand Analysis
    demandAnalyzer.analyzeAll(inventory, salesRecord, analysisPeriodDays);
    // Branch B: Expiry Analysis
    expiryManager.getAllExpiryStatus(inventory, simulatedCurrentDate);
    // Branch C: Shelf Space Analysis
    shelfManager.generateShelfReports();
    shelfManager.suggestRearrangements();
}
// Step 4: Reorder Analysis
void SmartShop::reorderAnalysis() {
    reorderManager.checkAllReorders(inventory, suppliers, demandAnalyzer, salesRecord, analysisPeriodDays);
}
// Step 5: Profit Analysis
void SmartShop::profitAnalysis() {
    profitAnalyzer.analyzeProfits(inventory, salesRecord);
}
// Step 6: Generate Smart Alerts & Recommendations
void SmartShop::generateAlerts() {
    currentAlerts.clear();
    // 1. Expired products check (HIGH severity)
    std::vector<ExpiryItem> expired = expiryManager.listExpired(inventory, simulatedCurrentDate);
    for (int i = 0; i < (int)expired.size(); i++) {
        SmartAlert alert;
        alert.type = "EXPIRED";
        alert.severity = "HIGH";
        alert.sku = expired[i].sku;
        alert.message = "Product " + expired[i].sku + " (" + expired[i].productName +
                        ") has EXPIRED on " + formatDate(expired[i].expiryDate) +
                        "! Quantity to discard: " + std::to_string(expired[i].quantity);
        currentAlerts.push_back(alert);
    }
    // 2. Expiring soon check (MEDIUM severity)
    std::vector<ExpiryItem> expiringSoon = expiryManager.listExpiringSoon(inventory, simulatedCurrentDate);
    for (int i = 0; i < (int)expiringSoon.size(); i++) {
        SmartAlert alert;
        alert.type = "EXPIRY_RISK";
        alert.severity = "MEDIUM";
        alert.sku = expiringSoon[i].sku;
        alert.message = "Batch of " + expiringSoon[i].sku + " (" + expiringSoon[i].productName +
                        ") expires in " + std::to_string(expiringSoon[i].daysLeft) +
                        " days! Qty: " + std::to_string(expiringSoon[i].quantity) + ". Apply discount.";
        currentAlerts.push_back(alert);
    }
    // 3. Reorder needed check (HIGH severity)
    std::vector<ReorderItem> reorders = reorderManager.checkAllReorders(inventory, suppliers, demandAnalyzer, salesRecord, analysisPeriodDays);
    limitToShelfSpace(reorders);
    for (int i = 0; i < (int)reorders.size(); i++) {
        if (reorders[i].needsReorder) {
            SmartAlert alert;
            alert.type = "REORDER";
            alert.severity = "HIGH";
            alert.sku = reorders[i].sku;
            alert.message = "Stock (" + std::to_string(reorders[i].currentStock) +
                            ") is at or below ROP (" + std::to_string((int)reorders[i].reorderPoint) +
                            ") for " + reorders[i].sku + ". Suggested order: " +
                            std::to_string(reorders[i].suggestedQuantity) + " units.";
            currentAlerts.push_back(alert);
        }
    }
    // 4. Low stock check (MEDIUM severity)
    std::vector<std::string> lowStock = inventory.checkLowStock(10);
    for (int i = 0; i < (int)lowStock.size(); i++) {
        Product* p = inventory.findProduct(lowStock[i]);
        if (p != nullptr) {
            SmartAlert alert;
            alert.type = "LOW_STOCK";
            alert.severity = "MEDIUM";
            alert.sku = lowStock[i];
            alert.message = "Low stock alert: " + lowStock[i] + " (" + p->getName() +
                            ") has only " + std::to_string(p->getTotalStock()) + " units remaining.";
            currentAlerts.push_back(alert);
        }
    }
    // 5. Overstock check (LOW severity)
    std::vector<std::string> overstocked = inventory.checkOverstock(80);
    for (int i = 0; i < (int)overstocked.size(); i++) {
        Product* p = inventory.findProduct(overstocked[i]);
        if (p != nullptr) {
            SmartAlert alert;
            alert.type = "OVERSTOCK";
            alert.severity = "LOW";
            alert.sku = overstocked[i];
            alert.message = "Overstock alert: " + overstocked[i] + " (" + p->getName() +
                            ") holds " + std::to_string(p->getTotalStock()) + " units.";
            currentAlerts.push_back(alert);
        }
    }
}
// Step 7: Save Data to text files
void SmartShop::saveData() {
    // 1. Save Shelves
    std::ofstream sfile("shelves.txt");
    const std::vector<Shelf>& shelves = shelfManager.getShelves();
    sfile << shelves.size() << "\n";
    for (int i = 0; i < (int)shelves.size(); i++) {
        std::string safeShelfName = shelves[i].getName();
        for (int c = 0; c < (int)safeShelfName.size(); c++) {
            if (safeShelfName[c] == ' ') {
                safeShelfName[c] = '_';
            }
        }
        sfile << shelves[i].getShelfId() << " "
              << safeShelfName << " "
              << shelves[i].getCapacity() << " "
              << shelves[i].getUsedSpace() << "\n";
    }
    sfile.close();
    // 2. Save Products and Batches
    std::ofstream pfile("products.txt");
    std::vector<Product> products = inventory.getAllProducts();
    pfile << products.size() << "\n";
    for (int i = 0; i < (int)products.size(); i++) {
        std::string safeName = products[i].getName();
        for (int c = 0; c < (int)safeName.size(); c++) {
            if (safeName[c] == ' ') {
                safeName[c] = '_';
            }
        }
        std::vector<Batch> batches = products[i].getBatches();
        pfile << products[i].getSku() << " "
              << safeName << " "
              << products[i].getCostPrice() << " "
              << products[i].getSellingPrice() << " "
              << products[i].getShelfId() << " "
              << products[i].getSpacePerUnit() << " "
              << batches.size();
        for (int b = 0; b < (int)batches.size(); b++) {
            pfile << " " << batches[b].quantity << " " << batches[b].expiryDate;
        }
        pfile << "\n";
    }
    pfile.close();
    // 3. Save Sales History
    std::ofstream salefile("sales.txt");
    std::vector<SaleEntry> sales = salesRecord.getAllSales();
    salefile << sales.size() << "\n";
    for (int i = 0; i < (int)sales.size(); i++) {
        salefile << sales[i].sku << " "
                 << sales[i].unitsSold << " "
                 << sales[i].dayNumber << "\n";
    }
    salefile.close();
    // 4. Save Suppliers
    std::ofstream supfile("suppliers.txt");
    supfile << suppliers.size() << "\n";
    for (int i = 0; i < (int)suppliers.size(); i++) {
        std::string safeSup = suppliers[i].getSupplierName();
        for (int c = 0; c < (int)safeSup.size(); c++) {
            if (safeSup[c] == ' ') {
                safeSup[c] = '_';
            }
        }
        supfile << suppliers[i].getSku() << " "
                << safeSup << " "
                << suppliers[i].getLeadTime() << " "
                << suppliers[i].getSafetyStock() << "\n";
    }
    supfile.close();
    std::cout << "Data saved successfully to text files.\n";
}
// Runs the full sequential workflow
void SmartShop::runWorkflow() {
    std::cout << "\n================ STARTING SMARTSHOP WORKFLOW ================\n";
    loadData();
    manageInventory();
    runAnalysisEngine();
    reorderAnalysis();
    profitAnalysis();
    generateAlerts();
    saveData();
    std::cout << "================ WORKFLOW COMPLETED SUCCESSFULLY ================\n\n";
}
// Returns structured product list for web API and console
std::vector<ProductReport> SmartShop::getProductList() const {
    std::vector<ProductReport> reports;
    std::vector<Product> prods = inventory.getAllProducts();
    for (int i = 0; i < (int)prods.size(); i++) {
        ProductReport pr;
        pr.sku = prods[i].getSku();
        pr.name = prods[i].getName();
        pr.costPrice = prods[i].getCostPrice();
        pr.sellingPrice = prods[i].getSellingPrice();
        pr.shelfId = prods[i].getShelfId();
        pr.spacePerUnit = prods[i].getSpacePerUnit();
        pr.shelfName = getShelfName(pr.shelfId);
        pr.totalStock = prods[i].getTotalStock();
        pr.batches = prods[i].getBatches();
        reports.push_back(pr);
    }
    return reports;
}
// Returns shelf reports
std::vector<ShelfReport> SmartShop::getShelfReport() const {
    return shelfManager.generateShelfReports();
}
// Returns shelf rearrangement suggestions
std::vector<ShelfSuggestion> SmartShop::getShelfRearrangements() const {
    return shelfManager.suggestRearrangements();
}
// Returns demand analysis report
std::vector<DemandItem> SmartShop::getDemandReport() const {
    return demandAnalyzer.analyzeAll(inventory, salesRecord, analysisPeriodDays);
}
// Returns FEFO expiry status report
std::vector<ExpiryItem> SmartShop::getExpiryReport() const {
    return expiryManager.getAllExpiryStatus(inventory, simulatedCurrentDate);
}
// Returns reorder recommendations report
std::vector<ReorderItem> SmartShop::getReorderReport() const {
    std::vector<ReorderItem> reorders = reorderManager.checkAllReorders(inventory, suppliers, demandAnalyzer, salesRecord, analysisPeriodDays);
    limitToShelfSpace(reorders);
    return reorders;
}
// Returns profit analysis summary
ProfitSummary SmartShop::getProfitReport() const {
    return profitAnalyzer.analyzeProfits(inventory, salesRecord);
}
// Returns generated smart alerts
std::vector<SmartAlert> SmartShop::getAlerts() const {
    return currentAlerts;
}
// Adds a new product to inventory with its initial delivery batch
bool SmartShop::addProduct(const std::string& sku, const std::string& name,
                          double costPrice, double sellingPrice, int shelfId,
                          int spacePerUnit, int initialQty, int expiryDate) {
    if (sku.empty() || name.empty() || costPrice < 0.0 || sellingPrice < 0.0 || initialQty <= 0 || spacePerUnit < 1) {
        std::cout << "Invalid product parameters!\n";
        return false;
    }
    // Verify shelf exists and can fit quantity
    Shelf* s = shelfManager.findShelf(shelfId);
    if (s == nullptr) {
        std::cout << "Shelf ID " << shelfId << " not found!\n";
        return false;
    }
    int spaceNeeded = initialQty * spacePerUnit;
    if (!s->canFit(spaceNeeded)) {
        std::cout << "Shelf " << shelfId << " cannot fit " << initialQty << " units (" << spaceNeeded << " space)! Free space: " << s->freeSpace() << "\n";
        return false;
    }
    Product newProd(sku, name, costPrice, sellingPrice, shelfId, spacePerUnit);
    newProd.addBatch(initialQty, expiryDate);
    bool added = inventory.addProduct(newProd);
    if (!added) {
        std::cout << "Product with SKU " << sku << " already exists!\n";
        return false;
    }
    s->addUsed(spaceNeeded);
    std::cout << "Product " << sku << " added successfully!\n";
    return true;
}
// Adds a delivery batch to an existing product
bool SmartShop::addBatchToProduct(const std::string& sku, int quantity, int expiryDate) {
    if (quantity <= 0 || expiryDate <= 0) {
        std::cout << "Invalid quantity or expiry date!\n";
        return false;
    }
    Product* p = inventory.findProduct(sku);
    if (p == nullptr) {
        std::cout << "Product with SKU " << sku << " not found!\n";
        return false;
    }
    Shelf* s = shelfManager.findShelf(p->getShelfId());
    if (s == nullptr) {
        std::cout << "Shelf " << p->getShelfId() << " not found!\n";
        return false;
    }
    int spaceNeeded = quantity * p->getSpacePerUnit();
    if (!s->canFit(spaceNeeded)) {
        std::cout << "Shelf " << p->getShelfId() << " cannot fit " << quantity << " units (" << spaceNeeded
                  << " space)! Free space: " << s->freeSpace() << "\n";
        return false;
    }
    bool added = p->addBatch(quantity, expiryDate);
    if (added) {
        s->addUsed(spaceNeeded);
        std::cout << "Received " << quantity << " units of " << sku << " (expiry " << formatDate(expiryDate) << ").\n";
    }
    return added;
}
// Records a sale transaction
bool SmartShop::recordSale(const std::string& sku, int unitsSold, int dayNumber) {
    bool success = salesRecord.recordSale(sku, unitsSold, dayNumber, inventory, shelfManager.getShelves());
    if (success) {
        generateAlerts();
    }
    return success;
}
