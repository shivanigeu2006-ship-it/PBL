# SmartStock

Retail inventory system with FEFO expiry control, demand analysis, reorder planning,
shelf management and profit reports. C++ core, console menu, and a web dashboard.

## Folder layout

    Product / Shelf / Inventory / SalesRecord / Supplier / DemandAnalyzer /
    ExpiryManager / ReorderManager / ShelfManager / ProfitAnalyzer / SmartShop  (.h + .cpp)
    main.cpp      console menu
    server.cpp    REST API + static file server (port 8080)
    frontend/     index.html, style.css, app.js
    Makefile, download_deps.sh

## Console version (no dependencies)

    make console
    ./smartstock

## Web version

    ./download_deps.sh     # fetches httplib.h and json.hpp (or download them manually)
    make server
    ./smartstock_server    # run from this folder so ./frontend is found
    # open http://localhost:8080

Needs g++ with C++17 support. On Windows with MinGW, add `-lws2_32` to the server link line.

Shelf space: every product has a "space per unit" (1 = small packet, 2+ = bigger items). A shelf's
capacity is in these space units, so 10 units of a space-2 item use 20 of the shelf.
Add shelves from the Shelves page (web) or menu option 12 (console).

Shelf names: every shelf has a name such as Dairy, Frozen or General. A product takes the name of
the shelf it is placed on and can only move to a shelf with the same name (Dairy to Dairy). When a
shelf is 80% full the Shelves page shows a tip with an Apply button (console: option 15). Use "Move a
Product to Another Shelf" (console: option 14) to move one by hand. If no same-name shelf has room,
add another one (for example a second Frozen shelf).

Restocking: the Reorder Plan page has a "Reorder" button per product and a Receive Stock form
(console: menu option 13). Each delivery is added as a new batch with its own expiry date.

Dates are shown and entered as YYYY-MM-DD (e.g. 2026-08-15).

Prices are in rupees (₹). If you ran an earlier version, delete the .txt data files first
(the products.txt format gained a space-per-unit column).

Data is saved to products.txt, shelves.txt, sales.txt and suppliers.txt in the folder
you run from. Delete them to reset to the built-in sample data.
