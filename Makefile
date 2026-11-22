CXX      = g++
CXXFLAGS = -std=c++17 -Wall -Wextra -O2
CORE     = Product.cpp Shelf.cpp Inventory.cpp SalesRecord.cpp Supplier.cpp \
           DemandAnalyzer.cpp ExpiryManager.cpp ReorderManager.cpp \
           ShelfManager.cpp ProfitAnalyzer.cpp SmartShop.cpp

console: $(CORE) main.cpp
	$(CXX) $(CXXFLAGS) $(CORE) main.cpp -o smartstock

server: $(CORE) server.cpp httplib.h json.hpp
	$(CXX) $(CXXFLAGS) $(CORE) server.cpp -o smartstock_server -pthread

clean:
	rm -f smartstock smartstock_server

.PHONY: console server clean
