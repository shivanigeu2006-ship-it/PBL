

#include "SmartShop.h"
#include "httplib.h"
#include "json.hpp"
using json = nlohmann::json;
int main() {
    
    SmartShop shop;
    shop.loadData();
    shop.generateAlerts();
    httplib::Server svr;
    
    svr.set_mount_point("/", "./frontend");
    
    svr.Get("/api/products", [&](const httplib::Request&, httplib::Response& res) {
        std::vector<ProductReport> prods = shop.getProductList();
        json jList = json::array();
        for (int i = 0; i < (int)prods.size(); i++) {
            json item;
            item["sku"] = prods[i].sku;
            item["name"] = prods[i].name;
            item["costPrice"] = prods[i].costPrice;
            item["sellingPrice"] = prods[i].sellingPrice;
            item["shelfId"] = prods[i].shelfId;
            item["shelfName"] = prods[i].shelfName;
            item["spacePerUnit"] = prods[i].spacePerUnit;
            item["totalStock"] = prods[i].totalStock;
            json bArray = json::array();
            for (int b = 0; b < (int)prods[i].batches.size(); b++) {
                json bObj;
                bObj["quantity"] = prods[i].batches[b].quantity;
                bObj["expiryDate"] = prods[i].batches[b].expiryDate;
                bArray.push_back(bObj);
            }
            item["batches"] = bArray;
            jList.push_back(item);
        }
        res.set_content(jList.dump(), "application/json");
    });
    
    svr.Post("/api/products", [&](const httplib::Request& req, httplib::Response& res) {
        json responseObj;
        try {
            json body = json::parse(req.body);
            std::string sku = body.value("sku", "");
            std::string name = body.value("name", "");
            double costPrice = body.value("costPrice", 0.0);
            double sellingPrice = body.value("sellingPrice", 0.0);
            int shelfId = body.value("shelfId", 1);
            int spacePerUnit = body.value("spacePerUnit", 1);
            int initialQty = body.value("initialQty", 0);
            int expiryDate = body.value("expiryDate", 0);
            bool success = shop.addProduct(sku, name, costPrice, sellingPrice, shelfId, spacePerUnit, initialQty, expiryDate);
            if (success) {
                shop.generateAlerts();
                shop.saveData();
                responseObj["success"] = true;
                responseObj["message"] = "Product added successfully!";
            } else {
                responseObj["success"] = false;
                responseObj["message"] = "Failed to add product. Check SKU uniqueness, shelf capacity (units x space per unit), and positive numbers.";
            }
        } catch (...) {
            responseObj["success"] = false;
            responseObj["message"] = "Invalid JSON payload!";
        }
        res.set_content(responseObj.dump(), "application/json");
    });
    
    svr.Post("/api/batches", [&](const httplib::Request& req, httplib::Response& res) {
        json responseObj;
        try {
            json body = json::parse(req.body);
            std::string sku = body.value("sku", "");
            int quantity = body.value("quantity", 0);
            int expiryDate = body.value("expiryDate", 0);
            bool success = shop.addBatchToProduct(sku, quantity, expiryDate);
            if (success) {
                shop.generateAlerts();
                shop.saveData();
                responseObj["success"] = true;
                responseObj["message"] = "Stock received: " + std::to_string(quantity) + " units added to " + sku + ".";
            } else {
                responseObj["success"] = false;
                responseObj["message"] = "Could not add stock. Check SKU, quantity, expiry date and shelf space.";
            }
        } catch (...) {
            responseObj["success"] = false;
            responseObj["message"] = "Invalid JSON payload!";
        }
        res.set_content(responseObj.dump(), "application/json");
    });
    
    svr.Post("/api/sales", [&](const httplib::Request& req, httplib::Response& res) {
        json responseObj;
        try {
            json body = json::parse(req.body);
            std::string sku = body.value("sku", "");
            int unitsSold = body.value("unitsSold", 0);
            int dayNumber = body.value("dayNumber", 1);
            bool success = shop.recordSale(sku, unitsSold, dayNumber);
            if (success) {
                shop.generateAlerts();
                shop.saveData();
                responseObj["success"] = true;
                responseObj["message"] = "Sale recorded successfully!";
            } else {
                responseObj["success"] = false;
                responseObj["message"] = "Sale rejected. Check SKU and ensure stock is sufficient.";
            }
        } catch (...) {
            responseObj["success"] = false;
            responseObj["message"] = "Invalid JSON payload!";
        }
        res.set_content(responseObj.dump(), "application/json");
    });
    
    svr.Get("/api/shelves", [&](const httplib::Request&, httplib::Response& res) {
        std::vector<ShelfReport> sReports = shop.getShelfReport();
        std::vector<ShelfSuggestion> suggestions = shop.getShelfRearrangements();
        json jShelves = json::array();
        for (int i = 0; i < (int)sReports.size(); i++) {
            json sObj;
            sObj["shelfId"] = sReports[i].shelfId;
            sObj["name"] = sReports[i].name;
            sObj["capacity"] = sReports[i].capacity;
            sObj["usedSpace"] = sReports[i].usedSpace;
            sObj["freeSpace"] = sReports[i].freeSpace;
            sObj["utilizationPercent"] = sReports[i].utilizationPercent;
            jShelves.push_back(sObj);
        }
        json jTips = json::array();
        for (int i = 0; i < (int)suggestions.size(); i++) {
            json tObj;
            tObj["fromShelfId"] = suggestions[i].fromShelfId;
            tObj["toShelfId"] = suggestions[i].toShelfId;
            tObj["message"] = suggestions[i].message;
            jTips.push_back(tObj);
        }
        json result;
        result["shelves"] = jShelves;
        result["suggestions"] = jTips;
        res.set_content(result.dump(), "application/json");
    });
    
    svr.Post("/api/shelves", [&](const httplib::Request& req, httplib::Response& res) {
        json responseObj;
        try {
            json body = json::parse(req.body);
            std::string name = body.value("name", "");
            int capacity = body.value("capacity", 0);
            int newId = shop.addNewShelf(name, capacity);
            if (newId > 0) {
                shop.saveData();
                responseObj["success"] = true;
                responseObj["shelfId"] = newId;
                responseObj["message"] = "Shelf " + std::to_string(newId) + " (" + name + ") added successfully!";
            } else {
                responseObj["success"] = false;
                responseObj["message"] = "Failed to add shelf. Give it a name and a positive capacity.";
            }
        } catch (...) {
            responseObj["success"] = false;
            responseObj["message"] = "Invalid JSON payload!";
        }
        res.set_content(responseObj.dump(), "application/json");
    });
    
    svr.Post("/api/move", [&](const httplib::Request& req, httplib::Response& res) {
        json responseObj;
        try {
            json body = json::parse(req.body);
            std::string sku = body.value("sku", "");
            int toShelfId = body.value("toShelfId", 0);
            bool success = shop.moveProductToShelf(sku, toShelfId);
            if (success) {
                shop.generateAlerts();
                shop.saveData();
            }
            responseObj["success"] = success;
            responseObj["message"] = shop.getLastMessage();
        } catch (...) {
            responseObj["success"] = false;
            responseObj["message"] = "Invalid JSON payload!";
        }
        res.set_content(responseObj.dump(), "application/json");
    });
    
    svr.Post("/api/apply-suggestion", [&](const httplib::Request& req, httplib::Response& res) {
        json responseObj;
        try {
            json body = json::parse(req.body);
            int fromShelfId = body.value("fromShelfId", 0);
            int toShelfId = body.value("toShelfId", 0);
            bool success = shop.applyRearrangement(fromShelfId, toShelfId);
            if (success) {
                shop.generateAlerts();
                shop.saveData();
            }
            responseObj["success"] = success;
            responseObj["message"] = shop.getLastMessage();
        } catch (...) {
            responseObj["success"] = false;
            responseObj["message"] = "Invalid JSON payload!";
        }
        res.set_content(responseObj.dump(), "application/json");
    });
    
    svr.Get("/api/demand", [&](const httplib::Request&, httplib::Response& res) {
        std::vector<DemandItem> demandList = shop.getDemandReport();
        json jDemand = json::array();
        for (int i = 0; i < (int)demandList.size(); i++) {
            json dObj;
            dObj["sku"] = demandList[i].sku;
            dObj["productName"] = demandList[i].productName;
            dObj["totalUnitsSold"] = demandList[i].totalUnitsSold;
            dObj["averageDailyDemand"] = demandList[i].averageDailyDemand;
            dObj["demandCategory"] = demandList[i].demandCategory;
            jDemand.push_back(dObj);
        }
        res.set_content(jDemand.dump(), "application/json");
    });
    
    svr.Get("/api/expiry", [&](const httplib::Request&, httplib::Response& res) {
        std::vector<ExpiryItem> expList = shop.getExpiryReport();
        json jExpiry = json::array();
        for (int i = 0; i < (int)expList.size(); i++) {
            json eObj;
            eObj["sku"] = expList[i].sku;
            eObj["productName"] = expList[i].productName;
            eObj["quantity"] = expList[i].quantity;
            eObj["expiryDate"] = expList[i].expiryDate;
            eObj["daysLeft"] = expList[i].daysLeft;
            eObj["status"] = expList[i].status;
            jExpiry.push_back(eObj);
        }
        res.set_content(jExpiry.dump(), "application/json");
    });
    
    svr.Get("/api/reorder", [&](const httplib::Request&, httplib::Response& res) {
        std::vector<ReorderItem> rList = shop.getReorderReport();
        json jReorder = json::array();
        for (int i = 0; i < (int)rList.size(); i++) {
            json rObj;
            rObj["sku"] = rList[i].sku;
            rObj["productName"] = rList[i].productName;
            rObj["currentStock"] = rList[i].currentStock;
            rObj["avgDailyDemand"] = rList[i].avgDailyDemand;
            rObj["leadTime"] = rList[i].leadTime;
            rObj["safetyStock"] = rList[i].safetyStock;
            rObj["reorderPoint"] = rList[i].reorderPoint;
            rObj["targetStock"] = rList[i].targetStock;
            rObj["suggestedQuantity"] = rList[i].suggestedQuantity;
            rObj["needsReorder"] = rList[i].needsReorder;
            rObj["limitedByShelf"] = rList[i].limitedByShelf;
            jReorder.push_back(rObj);
        }
        res.set_content(jReorder.dump(), "application/json");
    });
    
    svr.Get("/api/profit", [&](const httplib::Request&, httplib::Response& res) {
        ProfitSummary pSummary = shop.getProfitReport();
        json jItems = json::array();
        for (int i = 0; i < (int)pSummary.items.size(); i++) {
            json pObj;
            pObj["sku"] = pSummary.items[i].sku;
            pObj["productName"] = pSummary.items[i].productName;
            pObj["costPrice"] = pSummary.items[i].costPrice;
            pObj["sellingPrice"] = pSummary.items[i].sellingPrice;
            pObj["unitsSold"] = pSummary.items[i].unitsSold;
            pObj["profitPerUnit"] = pSummary.items[i].profitPerUnit;
            pObj["totalProfit"] = pSummary.items[i].totalProfit;
            pObj["profitMarginPercent"] = pSummary.items[i].profitMarginPercent;
            jItems.push_back(pObj);
        }
        json result;
        result["items"] = jItems;
        result["grandTotalProfit"] = pSummary.grandTotalProfit;
        result["averageMargin"] = pSummary.averageMargin;
        res.set_content(result.dump(), "application/json");
    });
    
    svr.Get("/api/alerts", [&](const httplib::Request&, httplib::Response& res) {
        shop.generateAlerts();
        std::vector<SmartAlert> alerts = shop.getAlerts();
        json jAlerts = json::array();
        for (int i = 0; i < (int)alerts.size(); i++) {
            json aObj;
            aObj["type"] = alerts[i].type;
            aObj["severity"] = alerts[i].severity;
            aObj["sku"] = alerts[i].sku;
            aObj["message"] = alerts[i].message;
            jAlerts.push_back(aObj);
        }
        res.set_content(jAlerts.dump(), "application/json");
    });
    
    svr.Post("/api/save", [&](const httplib::Request&, httplib::Response& res) {
        shop.saveData();
        json responseObj;
        responseObj["success"] = true;
        responseObj["message"] = "Data persisted successfully to files!";
        res.set_content(responseObj.dump(), "application/json");
    });
    std::cout << "=======================================================\n";
    std::cout << " SmartStock Web Server running at: http://localhost:8080\n";
    std::cout << " Press Ctrl+C to stop the server.\n";
    std::cout << "=======================================================\n";
    svr.listen("0.0.0.0", 8080);
    return 0;
}
