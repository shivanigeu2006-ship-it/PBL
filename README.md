# SmartStock

**A Retailer-Focused System for Demand-Aware Inventory and FEFO Expiry Control**
Team DSCPP-III-2026-T118 (C++17, OOP + STL data structures + file handling, with a web UI)

SmartStock is one C++ program. It contains all 11 modules from the proposal, a REST API, and a browser UI that the same program serves. There is nothing else to install.

```
 Browser UI (frontend/index.html)  <-- HTTP + JSON -->  main.cpp (REST API, cpp-httplib)
                                                              |
                                                          SmartShop  (integration class)
        ┌─────────┬───────────┬──────────┬─────────┬───────────┬───────────┬───────────┐
     Inventory  ShelfManager  SalesRecord  Supplier  DemandAnalyzer  ExpiryManager  ReorderManager  ProfitAnalyzer
     (Product)   (Shelf)                                                (FEFO heap)
                                                              |
                                                     data/*.json  (file handling)
```

## 1. Run it

You need a C++17 compiler (g++ 8 or newer, or clang). No other libraries: `httplib.h` and `json.hpp` are included in `include/`.

| System | Command |
|---|---|
| Linux / macOS | `./build_and_run.sh` (or `make && ./smartstock`) |
| Windows (MinGW-w64 g++) | double-click `build_and_run.bat` |
| Any (CMake) | `cmake -B build && cmake --build build` then run `build/smartstock` from this folder |

Then open **http://localhost:8080**.

On the very first run the program creates a demo shop (22 products, 8 shelves, 4 suppliers, 30 days of sales, expired and expiring batches) so every screen has something to show. Use **Data → Replace all data with demo data** to reset, or delete the `data/` folder.

Options: `smartstock --port 9000 --data mydata --web frontend`.
Keep the `frontend` folder next to the program (the server finds `frontend/` or `../frontend/`).

## 2. What each module does (maps to the proposal)

| Proposal module | File | Responsibility |
|---|---|---|
| Product | `src/Product.h` | SKU, prices, target/safety stock, **batches** (qty + expiry each); FEFO `sell()` |
| Inventory | `src/Inventory.h` | `unordered_map<SKU, Product>` for O(1) lookup |
| Shelf | `src/Shelf.h` | capacity, used space, free space, % full |
| ShelfManager | `src/ShelfManager.h` | assign/move with capacity check, auto-assign by zone, rearrangement suggestions |
| SalesRecord | `src/SalesRecord.h` | stores sales, validates, updates stock after every sale |
| Supplier | `src/Supplier.h` | lead time, default safety stock |
| DemandAnalyzer | `src/DemandAnalyzer.h` | average daily demand; sorts products and labels High / Medium / Low |
| ExpiryManager | `src/ExpiryManager.h` | `priority_queue` (min-heap on expiry) → FEFO order, expired / critical / soon |
| ReorderManager | `src/ReorderManager.h` | reorder point, suggested qty, low / out / overstock status, shelf-space check |
| ProfitAnalyzer | `src/ProfitAnalyzer.h` | profit per unit, total profit, margin, wastage and net profit |
| SmartShop | `src/SmartShop.h` | composes everything, file handling (save/load), smart alerts, demo data |
| (new) API + server | `src/main.cpp` | REST endpoints, JSON errors, CSV export, serves the UI |
| (new) UI | `frontend/index.html` | 11 screens, plain HTML/CSS/JS |

OOP concepts: **encapsulation** (private data, public methods), **abstraction** (`SmartShop` hides the analysis), **composition** (`SmartShop` owns every module), **constructors**.
Data structures: `unordered_map` (SKU lookup), `priority_queue` (FEFO), `vector` arrays, `std::sort`/`stable_sort` (demand ranking), `map`.

### Formulas (exactly as in the proposal)

- Average Daily Demand = Total Units Sold / Number of Days
- Reorder Point = Average Daily Demand × Lead Time + Safety Stock
- Suggested Order Quantity = Target Stock − Current Stock
- Profit per Unit = Selling Price − Cost Price
- Total Profit = Profit per Unit × Units Sold (summed per sale, so old sales keep the price they were sold at)
- Profit Margin = (Selling Price − Cost Price) / Selling Price × 100

### Rules and assumptions

- **FEFO**: a sale takes units from the batch with the earliest expiry. Expired batches are never sold.
- A batch is expired the day *after* its expiry date. Non-perishables have no expiry date.
- "Current stock" for reorder purposes counts only sellable (not expired) units.
- Status: `out_of_stock` (0 sellable), `reorder` (stock ≤ reorder point), `overstock` (stock > 1.5 × target), else `ok`.
- Expiry status: `expired`, `critical` (≤ 1 day), `soon` (within the chosen days, default 7), `ok`.
- Demand classes: products are sorted by units sold; top third = High, middle = Medium, rest = Low. Nothing sold = Low.
  If the shop has less history than the look-back window, the history length is used, so new shops are not under-counted. `days = 0` is safe.
- Space model: one unit of stock uses one unit of shelf capacity. Adding stock beyond free space is rejected (HTTP 409).
- Safety stock: a product's own value if set, otherwise its supplier's default.
- Shelf suggestions only move products to a shelf of the matching zone or a `General` shelf, never e.g. milk to the household wall.
- EOQ is left as a future enhancement, as stated in the proposal.

## 3. REST API

All bodies are JSON. Errors look like `{"error": "message"}` with a proper status (400 invalid input, 404 unknown id, 409 conflict such as not enough stock or shelf space).

| Method & path | Purpose |
|---|---|
| `GET /api/dashboard` | summary numbers, first alerts, 14-day sales, top demand |
| `GET /api/alerts` | all smart alerts (critical / warning / info) |
| `GET /api/products?q=` | list with stock, status, demand level, batches |
| `POST /api/products` | add (`sku, name, cost, price, target`, optional `category, supplierId, shelfId, safety, initialQty, expiry`) |
| `GET / PUT / DELETE /api/products/{sku}` | read, update, delete |
| `POST /api/products/{sku}/batches` | receive stock `{qty, expiry?}` |
| `POST /api/sales` | record a sale `{sku, units, date?}` (FEFO) |
| `GET /api/sales?sku=&limit=` | sales history, newest first |
| `GET / POST /api/suppliers`, `PUT / DELETE /api/suppliers/{id}` | suppliers |
| `GET / POST /api/shelves`, `PUT / DELETE /api/shelves/{id}` | shelves with contents + suggestions |
| `POST /api/shelves/assign` | `{sku, shelfId}` move/assign with capacity check |
| `POST /api/shelves/auto-assign` | place every unassigned product |
| `GET /api/demand?days=30` | average daily demand and High/Medium/Low |
| `GET /api/expiry?days=7` | FEFO-ordered batches with status |
| `POST /api/expiry/discard` | write off expired stock `{sku?}` (recorded as wastage) |
| `GET /api/reorder?days=30` | reorder point, suggested/order qty, supplier, shelf-space warning |
| `GET /api/profit`, `GET /api/wastage` | profit per product + summary; wastage log |
| `GET /api/export/{products\|sales\|profit\|reorder}` | CSV download |
| `POST /api/seed` | `{"confirm": true}` reload demo data |

Quick check: `curl http://localhost:8080/api/alerts`

## 4. Data files (file handling)

Saved in `data/` as readable JSON after every change (written to a temp file and then renamed, so a crash cannot leave a half-written file): `products.json` (with batches), `shelves.json`, `suppliers.json`, `sales.json`, `wastage.json`. They are loaded when the program starts. Each class has its own `toJson()` / `fromJson()` so the format of every class is fixed in one place. Shelf usage is never stored; it is recomputed from the stock, so it can never go out of sync.

## 5. Tests

```
make test                      # or run the two parts yourself:
g++ -std=c++17 -Iinclude -Isrc tests/test_core.cpp -pthread -o test_core && ./test_core
./smartstock --port 8099 --data /tmp/ss_test &  python3 tests/api_test.py 8099
```

`test_core.cpp` (41 checks): dates, FEFO selling, batches, capacity, demand, reorder formulas, expiry queue, profit, save/load.
`api_test.py` (38 checks): every endpoint, validation errors, capacity rejection, FEFO through the API, formulas, write-off, CSV.

## 6. Thread safety

The HTTP server handles requests on several threads. `SmartShop` guards all state with one `std::recursive_mutex`, so two browser tabs cannot corrupt the data.

## 7. Suggested text for the next report

- **Architecture**: add two boxes above SmartShop: *REST API layer (main.cpp)* and *Browser UI (frontend)*, joined by "HTTP + JSON". The analysis flow in the existing diagram does not change.
- **Concepts used**: add *REST API design, JSON serialisation, mutex-based thread safety, single-page web UI*.
- **Testing table**: replace "Pending" rows with the two test files above (shelf capacity, integration, file handling are all covered).
- **Challenges**: keeping console I/O out of the classes (they return data; the API prints nothing), and keeping shelf usage consistent (recomputed from stock instead of stored).

## 8. Folder layout

```
SmartStock/
  src/        all C++ modules (headers) + main.cpp
  include/    httplib.h, json.hpp (third-party, single-header)
  frontend/   index.html (the whole UI)
  data/       created at runtime
  tests/      test_core.cpp, api_test.py
  Makefile  CMakeLists.txt  build_and_run.sh  build_and_run.bat
```

Third-party: [cpp-httplib](https://github.com/yhirose/cpp-httplib) and [nlohmann/json](https://github.com/nlohmann/json), both MIT licensed.
