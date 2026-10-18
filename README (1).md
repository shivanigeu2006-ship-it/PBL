# Kashish Jain (2510012208)
SmartStock: A Retailer-Focused System for Demand-Aware Inventory and FEFO Expiry Control

## My modules
| File | What it does |
|---|---|
| `DemandAnalyzer.h` | Calculates average daily demand from sales, sorts products by demand and labels them High, Medium or Low (top third, middle third, rest; nothing sold = Low). Handles days = 0 and short sales history safely. |
| `ExpiryManager.h` | Finds expired and soon-to-expire batches. Uses a `priority_queue` (min-heap on expiry date) to list batches in FEFO order. Status: expired, critical, soon, ok. |
| `SmartShop.h` | The integration class. Owns every module (composition), connects the whole workflow, validates input, saves and loads all data with C++ file handling (JSON files), writes the demo data, and builds the dashboard. |

## Formula implemented
Average Daily Demand = Total Units Sold / Number of Days

## Concepts used
Composition, abstraction, `priority_queue` (O(log n)), sorting (O(n log n)), file handling (`fstream`), `std::mutex` for thread safety.

## File handling
Data is saved after every change to `products.json`, `shelves.json`, `suppliers.json`, `sales.json` and `wastage.json`. Each file is written to a temporary file and then renamed, so a crash cannot leave a half-written file.

## Note
These files are part of the full SmartStock project and depend on the other members' files, so they build together with it from the main project folder.
