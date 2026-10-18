# Palak Priya (2510012474)
SmartStock: A Retailer-Focused System for Demand-Aware Inventory and FEFO Expiry Control

## My modules
| File | What it does |
|---|---|
| `Product.h` | Product record identified by a unique SKU. Stock is kept in batches (quantity + expiry date each). `sell()` takes units from the earliest-expiring batch first (FEFO) and never sells expired units. Also gives profit per unit and margin. |
| `Inventory.h` | Stores all products in an `unordered_map` keyed by SKU for average O(1) lookup. Add, find, remove. |
| `Shelf.h` | One shelf: capacity, used space, free space and % full. |
| `ShelfManager.h` | Assigns products to shelves, rejects stock that does not fit, auto-assigns by matching shelf zone to product category, suggests moves when a shelf is 85% full or more. |
| `Utils.h` | Date helpers (yyyy-mm-dd to day number and back), rounding, shared error type. |

## Concepts used
Encapsulation (private data, public methods), constructors, `unordered_map`, vectors, sorting, STL algorithms.

## Key rules
- A batch is expired the day after its expiry date.
- One unit of stock uses one unit of shelf capacity.
- Shelf used space is recalculated from stock, never stored.

## Note
These files are part of the full SmartStock project and depend on the other members' files, so they build together with it from the main project folder.

