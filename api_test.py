#!/usr/bin/env python3
"""End-to-end API test for SmartStock. Needs only Python 3 (no packages).
Start the server on a scratch data folder first, e.g.:
    ./smartstock --port 8099 --data /tmp/ss_test
then run:   python3 tests/api_test.py 8099
"""
import json, sys, datetime, urllib.request, urllib.error

PORT = sys.argv[1] if len(sys.argv) > 1 else "8080"
BASE = f"http://localhost:{PORT}/api"
passed = failed = 0


def call(method, path, body=None, raw=False):
    data = None if body is None else (body if isinstance(body, bytes) else json.dumps(body).encode())
    req = urllib.request.Request(BASE + path, data=data, method=method, headers={"Content-Type": "application/json"})
    try:
        with urllib.request.urlopen(req) as r:
            txt = r.read().decode()
            return r.status, (txt if raw else json.loads(txt))
    except urllib.error.HTTPError as e:
        return e.code, json.loads(e.read().decode() or "{}")


def check(name, cond, extra=""):
    global passed, failed
    if cond:
        passed += 1
        print(f"  PASS  {name}")
    else:
        failed += 1
        print(f"  FAIL  {name} {extra}")


def day(offset):
    return (datetime.date.today() + datetime.timedelta(days=offset)).isoformat()


print("Products, batches and FEFO")
s, _ = call("POST", "/seed", {"confirm": True})
check("reset to demo data", s == 200)
s, p = call("POST", "/products", {"sku": "T-MILK", "name": "Test Milk", "category": "Dairy", "cost": 40, "price": 50,
                                  "target": 50, "supplierId": "SUP1", "shelfId": "SH-A1", "initialQty": 10, "expiry": day(10)})
check("create product with first batch", s == 201 and p["stock"] == 10, p)
s, p = call("POST", "/products/T-MILK/batches", {"qty": 5, "expiry": day(3)})
check("add earlier-expiring batch", s == 201 and p["stock"] == 15 and p["batches"][0]["expiry"] == day(3), p)
s, r = call("POST", "/sales", {"sku": "T-MILK", "units": 7})
check("sale succeeds", s == 201 and r["product"]["stock"] == 8, r)
b = r["product"]["batches"]
check("FEFO: earliest batch (5) emptied first, 2 taken from later batch",
      len(b) == 1 and b[0]["expiry"] == day(10) and b[0]["qty"] == 8, b)

print("Validation and errors")
s, r = call("POST", "/sales", {"sku": "T-MILK", "units": 999})
check("selling more than stock -> 409", s == 409 and "sellable" in r["error"], r)
s, r = call("POST", "/sales", {"sku": "NOPE", "units": 1})
check("unknown SKU sale -> 404", s == 404, r)
s, r = call("POST", "/sales", {"sku": "T-MILK", "units": 0})
check("zero units -> 400", s == 400, r)
s, r = call("POST", "/products", {"sku": "T-MILK", "name": "x", "cost": 1, "price": 2, "target": 5})
check("duplicate SKU -> 409", s == 409, r)
s, r = call("POST", "/products", {"sku": "T-BAD", "name": "x", "cost": -1, "price": 2, "target": 5})
check("negative cost -> 400", s == 400, r)
s, r = call("POST", "/products", b"{not json")
check("malformed JSON -> 400", s == 400, r)
s, r = call("POST", "/products/T-MILK/batches", {"qty": 1, "expiry": "2020-01-01"})
check("already-expired batch rejected", s == 400, r)
s, r = call("POST", "/products/T-MILK/batches", {"qty": 1, "expiry": "2026-02-31"})
check("impossible date rejected", s == 400, r)
s, r = call("GET", "/nothing-here")
check("unknown endpoint -> JSON 404", s == 404 and "error" in r, r)

print("Shelf capacity")
s, r = call("POST", "/products/T-MILK/batches", {"qty": 100000, "expiry": day(5)})
check("stock beyond shelf capacity rejected", s == 409 and "free space" in r["error"], r)
s, sh = call("GET", "/shelves")
check("shelves report used + free", all(x["used"] + x["free"] == x["capacity"] for x in sh["shelves"]))
s, r = call("POST", "/shelves/assign", {"sku": "T-MILK", "shelfId": "SH-E1"})
check("moving 8 units to 60-space shelf works", s == 200, r)
s, r = call("POST", "/shelves/assign", {"sku": "RICE-5KG", "shelfId": "SH-E1"})
check("moving 70 units onto a shelf that can't hold them -> 409", s == 409, r)
s, r = call("PUT", "/shelves/SH-E1", {"capacity": 5})
check("cannot shrink shelf below used space", s == 409, r)

print("Reorder and demand formulas")
s, ro = call("GET", "/reorder")
ok = True
for it in ro["items"]:
    exp_rop = round(it["avgDaily"] * it["leadTime"] + it["safety"], 1)
    if abs(exp_rop - it["reorderPoint"]) > 0.1 + it["leadTime"] * 0.006:
        ok = False
    if it["suggestedQty"] != max(0, it["target"] - it["current"]):
        ok = False
check("reorder point = avg x lead + safety; suggested = target - current", ok)
s, dm = call("GET", "/demand?days=30")
levels = {d["level"] for d in dm}
check("demand has High/Medium/Low", levels == {"High", "Medium", "Low"}, levels)
check("demand sorted high to low", all(dm[i]["totalSold"] >= dm[i + 1]["totalSold"] for i in range(len(dm) - 1)))
s, dm0 = call("GET", "/demand?days=0")
check("days=0 falls back safely (no divide by zero)", s == 200 and dm0[0]["days"] > 0)

print("Expiry, wastage and profit")
s, ex = call("GET", "/expiry")
check("expiry queue is FEFO ordered", [i["expiryDay"] for i in ex["items"]] == sorted(i["expiryDay"] for i in ex["items"]))
check("expired stock present in demo data", ex["expiredUnits"] > 0)
s, d = call("POST", "/expiry/discard", {})
check("discard expired writes off stock", s == 200 and d["units"] > 0 and d["loss"] > 0, d)
s, ex = call("GET", "/expiry")
check("no expired units remain", ex["expiredUnits"] == 0)
s, pf = call("GET", "/profit")
tot = round(sum(x["totalProfit"] for x in pf["products"]), 2)
check("profit summary matches sum of products", abs(tot - pf["summary"]["grossProfit"]) < 1, (tot, pf["summary"]))
check("net profit = gross - wastage", abs(pf["summary"]["grossProfit"] - pf["summary"]["wastageLoss"] - pf["summary"]["netProfit"]) < 0.02)
s, al = call("GET", "/alerts")
check("alerts generated", s == 200 and len(al) > 0)

print("Other endpoints")
s, r = call("GET", "/products?q=milk")
check("product search", s == 200 and any(x["sku"] == "T-MILK" for x in r))
s, r = call("PUT", "/products/T-MILK", {"price": 55})
check("update price", s == 200 and r["price"] == 55 and r["profitPerUnit"] == 15, r)
s, r = call("POST", "/suppliers", {"id": "SUP9", "name": "Test Supplier", "leadTime": 2, "safetyStock": 4})
check("add supplier", s == 201, r)
s, r = call("DELETE", "/suppliers/SUP1")
check("cannot delete supplier in use", s == 409, r)
s, r = call("DELETE", "/suppliers/SUP9")
check("delete unused supplier", s == 200, r)
s, csv = call("GET", "/export/products", raw=True)
check("CSV export", s == 200 and csv.startswith("sku,"))
s, r = call("DELETE", "/products/T-MILK")
check("delete product", s == 200)
s, r = call("GET", "/products/T-MILK")
check("deleted product gone", s == 404)

print(f"\n{passed} passed, {failed} failed")
sys.exit(1 if failed else 0)
