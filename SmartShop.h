#ifndef SMARTSHOP_H
#define SMARTSHOP_H
#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include "Product.h"
#include "Shelf.h"
#include "Inventory.h"
#include "SalesRecord.h"
#include "Supplier.h"
#include "DemandAnalyzer.h"
#include "ExpiryManager.h"
#include "ReorderManager.h"
#include "ShelfManager.h"
#include "ProfitAnalyzer.h"

struct SmartAlert {
    std::string type;     
    std::string severity; 
    std::string sku;
    std::string message;
};

class SmartShop {
private:
    Inventory inventory;
    ShelfManager shelfManager;
    SalesRecord salesRecord;
    std::vector<Supplier> suppliers;
    DemandAnalyzer demandAnalyzer;
    ExpiryManager expiryManager;
    ReorderManager reorderManager;
    ProfitAnalyzer profitAnalyzer;
    std::vector<SmartAlert> currentAlerts;
    int simulatedCurrentDate; 
    int analysisPeriodDays;   
    std::string lastMessage;  
    void limitToShelfSpace(std::vector<ReorderItem>& items) const;
    std::string getShelfName(int shelfId) const;
    void setMessage(const std::string& message);
public:
    SmartShop();
    
    void loadData();
    void manageInventory();
    void runAnalysisEngine();
    void reorderAnalysis();
    void profitAnalysis();
    void generateAlerts();
    void saveData();
    
    void runWorkflow();
    
    std::vector<ProductReport> getProductList() const;
    std::vector<ShelfReport> getShelfReport() const;
    std::vector<ShelfSuggestion> getShelfRearrangements() const;
    std::vector<DemandItem> getDemandReport() const;
    std::vector<ExpiryItem> getExpiryReport() const;
    std::vector<ReorderItem> getReorderReport() const;
    ProfitSummary getProfitReport() const;
    std::vector<SmartAlert> getAlerts() const;
    
    bool addProduct(const std::string& sku, const std::string& name,
                    double costPrice, double sellingPrice, int shelfId,
                    int spacePerUnit, int initialQty, int expiryDate);
    bool addBatchToProduct(const std::string& sku, int quantity, int expiryDate);
    bool recordSale(const std::string& sku, int unitsSold, int dayNumber);
    void addShelf(int shelfId, const std::string& name, int capacity);
    int addNewShelf(const std::string& name, int capacity); 
    bool moveProductToShelf(const std::string& sku, int newShelfId);
    bool applyRearrangement(int fromShelfId, int toShelfId); 
    std::string getLastMessage() const; 
    void addSupplier(const Supplier& supplier);
    void preloadSampleData();
    int getCurrentDate() const;
    void setCurrentDate(int date);
    int getAnalysisDays() const;
    void setAnalysisDays(int days);
};
#endif
