

function showPage(pageId) {
    
    var pages = document.querySelectorAll(".page-section");
    for (var i = 0; i < pages.length; i++) {
        pages[i].classList.remove("active");
    }
    
    var buttons = document.querySelectorAll(".nav-item");
    for (var j = 0; j < buttons.length; j++) {
        buttons[j].classList.remove("active");
    }
    
    var targetPage = document.getElementById("page-" + pageId);
    if (targetPage) {
        targetPage.classList.add("active");
    }
    
    var activeBtn = document.querySelector(".nav-item[onclick=\"showPage('" + pageId + "')\"]");
    if (activeBtn) {
        activeBtn.classList.add("active");
    }
    
    var titleMap = {
        "dashboard": "Dashboard Overview",
        "products": "Inventory & Products Catalog",
        "sales": "Record Customer Sales",
        "shelves": "Shelf Capacity & Space Allocation",
        "demand": "Demand Classification (High/Medium/Low)",
        "expiry": "FEFO Expiry Schedule",
        "reorder": "Reorder Point & Order Suggestions",
        "profit": "Profitability & Margin Analysis"
    };
    document.getElementById("page-title").innerText = titleMap[pageId] || "SmartStock";
    
    if (pageId === "dashboard") {
        loadDashboard();
    } else if (pageId === "products") {
        loadProducts();
    } else if (pageId === "shelves") {
        loadShelves();
    } else if (pageId === "demand") {
        loadDemand();
    } else if (pageId === "expiry") {
        loadExpiry();
    } else if (pageId === "reorder") {
        loadReorder();
    } else if (pageId === "profit") {
        loadProfit();
    }
}

function formatDate(num) {
    var s = String(num);
    if (s.length !== 8) {
        return s;
    }
    return s.substring(0, 4) + "-" + s.substring(4, 6) + "-" + s.substring(6, 8);
}

function escapeHtml(text) {
    var div = document.createElement("div");
    div.textContent = String(text);
    return div.innerHTML;
}

function showNotification(message, isError) {
    var bar = document.getElementById("notification-bar");
    bar.innerText = message;
    bar.className = "notification " + (isError ? "error" : "success");
    bar.classList.remove("hidden");
    setTimeout(function() {
        bar.classList.add("hidden");
    }, 4000);
}

function loadDashboard() {
    
    fetch("/api/alerts")
        .then(function(res) { return res.json(); })
        .then(function(alerts) {
            var container = document.getElementById("alerts-container");
            container.innerHTML = "";
            var criticalCount = 0;
            var reorderCount = 0;
            if (alerts.length === 0) {
                container.innerHTML = "<p style='color: #64748b;'>No alerts currently active. All stock levels and expiry dates are healthy!</p>";
            }
            for (var i = 0; i < alerts.length; i++) {
                var a = alerts[i];
                if (a.type === "EXPIRED" || a.type === "EXPIRY_RISK") {
                    criticalCount++;
                }
                if (a.type === "REORDER") {
                    reorderCount++;
                }
                var div = document.createElement("div");
                div.className = "alert-item alert-" + a.severity;
                div.innerHTML = "<strong>[" + a.severity + "]</strong> <span>" + a.message + "</span>";
                container.appendChild(div);
            }
            document.getElementById("kpi-expiry-count").innerText = criticalCount;
            document.getElementById("kpi-reorder-count").innerText = reorderCount;
        })
        .catch(function(err) {
            console.error("Error loading alerts:", err);
        });
    
    fetch("/api/products")
        .then(function(res) { return res.json(); })
        .then(function(products) {
            var total = 0;
            for (var i = 0; i < products.length; i++) {
                total += products[i].totalStock;
            }
            document.getElementById("kpi-total-stock").innerText = total;
        });
    
    fetch("/api/profit")
        .then(function(res) { return res.json(); })
        .then(function(data) {
            var profit = data.grandTotalProfit || 0;
            document.getElementById("kpi-total-profit").innerText = "₹" + profit.toFixed(2);
        });
}

function loadShelfOptions() {
    fetch("/api/shelves")
        .then(function(res) { return res.json(); })
        .then(function(data) {
            var shelves = data.shelves || [];
            var selectIds = ["prod-shelf", "mv-shelf"];
            for (var k = 0; k < selectIds.length; k++) {
                var select = document.getElementById(selectIds[k]);
                var previous = select.value;
                select.innerHTML = "";
                for (var i = 0; i < shelves.length; i++) {
                    var opt = document.createElement("option");
                    opt.value = shelves[i].shelfId;
                    opt.textContent = "Shelf " + shelves[i].shelfId + " - " + shelves[i].name +
                                      " (free " + shelves[i].freeSpace + " / " + shelves[i].capacity + ")";
                    select.appendChild(opt);
                }
                if (previous) {
                    select.value = previous;
                }
            }
        });
}

function handleMoveProduct(event) {
    event.preventDefault();
    var payload = {
        sku: document.getElementById("mv-sku").value.trim().toUpperCase(),
        toShelfId: parseInt(document.getElementById("mv-shelf").value)
    };
    fetch("/api/move", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    })
    .then(function(res) { return res.json(); })
    .then(function(data) {
        showNotification(data.message, !data.success);
        if (data.success) {
            document.getElementById("move-product-form").reset();
            loadShelves();
        }
    })
    .catch(function(err) {
        showNotification("Failed to connect to server.", true);
    });
}

function makeApplyHandler(fromShelfId, toShelfId) {
    return function() {
        fetch("/api/apply-suggestion", {
            method: "POST",
            headers: { "Content-Type": "application/json" },
            body: JSON.stringify({ fromShelfId: fromShelfId, toShelfId: toShelfId })
        })
        .then(function(res) { return res.json(); })
        .then(function(data) {
            showNotification(data.message, !data.success);
            loadShelves();
        })
        .catch(function(err) {
            showNotification("Failed to connect to server.", true);
        });
    };
}

function handleAddShelf(event) {
    event.preventDefault();
    var payload = {
        name: document.getElementById("shelf-name").value.trim(),
        capacity: parseInt(document.getElementById("shelf-capacity").value)
    };
    fetch("/api/shelves", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    })
    .then(function(res) { return res.json(); })
    .then(function(data) {
        if (data.success) {
            showNotification(data.message, false);
            document.getElementById("add-shelf-form").reset();
            loadShelves();
            loadShelfOptions();
        } else {
            showNotification(data.message, true);
        }
    })
    .catch(function(err) {
        showNotification("Failed to connect to server.", true);
    });
}

function loadProducts() {
    loadShelfOptions();
    fetch("/api/products")
        .then(function(res) { return res.json(); })
        .then(function(products) {
            var tbody = document.getElementById("products-table-body");
            tbody.innerHTML = "";
            for (var i = 0; i < products.length; i++) {
                var p = products[i];
                var tr = document.createElement("tr");
                
                var batchHtml = "";
                for (var b = 0; b < p.batches.length; b++) {
                    var batch = p.batches[b];
                    batchHtml += "<span class='badge badge-blue' style='margin-right: 4px; margin-bottom: 2px;'>" +
                                 batch.quantity + " units (Exp: " + formatDate(batch.expiryDate) + ")</span>";
                }
                if (p.batches.length === 0) {
                    batchHtml = "<span style='color:#94a3b8;'>No active batches</span>";
                }
                tr.innerHTML = "<td><strong>" + p.sku + "</strong></td>" +
                               "<td>" + p.name + "</td>" +
                               "<td>₹" + p.costPrice.toFixed(2) + "</td>" +
                               "<td>₹" + p.sellingPrice.toFixed(2) + "</td>" +
                               "<td>Shelf " + p.shelfId + " (" + escapeHtml(p.shelfName) + ")</td>" +
                               "<td>" + p.spacePerUnit + "</td>" +
                               "<td><strong>" + p.totalStock + "</strong></td>" +
                               "<td>" + batchHtml + "</td>";
                tbody.appendChild(tr);
            }
        });
}

function handleAddProduct(event) {
    event.preventDefault();
    var payload = {
        sku: document.getElementById("prod-sku").value.trim().toUpperCase(),
        name: document.getElementById("prod-name").value.trim(),
        costPrice: parseFloat(document.getElementById("prod-cp").value),
        sellingPrice: parseFloat(document.getElementById("prod-sp").value),
        shelfId: parseInt(document.getElementById("prod-shelf").value),
        spacePerUnit: parseInt(document.getElementById("prod-space").value),
        initialQty: parseInt(document.getElementById("prod-qty").value),
        expiryDate: parseInt(document.getElementById("prod-exp").value.replace(/-/g, ""))
    };
    fetch("/api/products", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    })
    .then(function(res) { return res.json(); })
    .then(function(data) {
        if (data.success) {
            showNotification(data.message, false);
            document.getElementById("add-product-form").reset();
            document.getElementById("prod-space").value = "1";
            loadProducts();
        } else {
            showNotification(data.message, true);
        }
    })
    .catch(function(err) {
        showNotification("Failed to send request to server.", true);
    });
}

function handleRecordSale(event) {
    event.preventDefault();
    var payload = {
        sku: document.getElementById("sale-sku").value.trim().toUpperCase(),
        unitsSold: parseInt(document.getElementById("sale-units").value),
        dayNumber: parseInt(document.getElementById("sale-day").value)
    };
    fetch("/api/sales", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    })
    .then(function(res) { return res.json(); })
    .then(function(data) {
        if (data.success) {
            showNotification(data.message, false);
            document.getElementById("record-sale-form").reset();
            document.getElementById("sale-day").value = "7";
        } else {
            showNotification(data.message, true);
        }
    })
    .catch(function(err) {
        showNotification("Failed to connect to server.", true);
    });
}

function loadShelves() {
    loadShelfOptions();
    fetch("/api/shelves")
        .then(function(res) { return res.json(); })
        .then(function(data) {
            var grid = document.getElementById("shelves-container");
            grid.innerHTML = "";
            var shelves = data.shelves || [];
            for (var i = 0; i < shelves.length; i++) {
                var s = shelves[i];
                var util = s.utilizationPercent;
                var colorClass = "";
                if (util >= 85) {
                    colorClass = "danger";
                } else if (util >= 70) {
                    colorClass = "warning";
                }
                var box = document.createElement("div");
                box.className = "shelf-box";
                box.innerHTML =
                    "<div class='shelf-header'>" +
                        "<h3>Shelf " + s.shelfId + " - " + escapeHtml(s.name) + "</h3>" +
                        "<span><strong>" + util.toFixed(1) + "%</strong> Full</span>" +
                    "</div>" +
                    "<div class='progress-bar-container'>" +
                        "<div class='progress-bar-fill " + colorClass + "' style='width: " + Math.min(100, util) + "%;'></div>" +
                    "</div>" +
                    "<div class='shelf-details'>" +
                        "<span>Used: " + s.usedSpace + " / " + s.capacity + " units</span>" +
                        "<span>Free: " + s.freeSpace + " units</span>" +
                    "</div>";
                grid.appendChild(box);
            }
            var tipsContainer = document.getElementById("shelf-tips-container");
            tipsContainer.innerHTML = "";
            var tips = data.suggestions || [];
            if (tips.length === 0) {
                tipsContainer.innerHTML = "<p style='color: #64748b;'>All shelves are balanced. No rearrangement required.</p>";
            }
            for (var t = 0; t < tips.length; t++) {
                var tipDiv = document.createElement("div");
                tipDiv.className = "tip-box";
                var tipText = document.createElement("span");
                tipText.innerText = "💡 " + tips[t].message;
                tipDiv.appendChild(tipText);
                
                if (tips[t].toShelfId > 0) {
                    var applyBtn = document.createElement("button");
                    applyBtn.className = "btn-primary btn-small";
                    applyBtn.textContent = "Apply";
                    applyBtn.onclick = makeApplyHandler(tips[t].fromShelfId, tips[t].toShelfId);
                    tipDiv.appendChild(applyBtn);
                }
                tipsContainer.appendChild(tipDiv);
            }
        });
}

function loadDemand() {
    fetch("/api/demand")
        .then(function(res) { return res.json(); })
        .then(function(items) {
            var tbody = document.getElementById("demand-table-body");
            tbody.innerHTML = "";
            for (var i = 0; i < items.length; i++) {
                var d = items[i];
                var tr = document.createElement("tr");
                var badgeClass = "badge-blue";
                if (d.demandCategory === "High") {
                    badgeClass = "badge-green";
                } else if (d.demandCategory === "Medium") {
                    badgeClass = "badge-orange";
                }
                tr.innerHTML = "<td><strong>" + d.sku + "</strong></td>" +
                               "<td>" + d.productName + "</td>" +
                               "<td>" + d.totalUnitsSold + " units</td>" +
                               "<td>" + d.averageDailyDemand.toFixed(2) + " units/day</td>" +
                               "<td><span class='badge " + badgeClass + "'>" + d.demandCategory + "</span></td>";
                tbody.appendChild(tr);
            }
        });
}

function loadExpiry() {
    fetch("/api/expiry")
        .then(function(res) { return res.json(); })
        .then(function(items) {
            var tbody = document.getElementById("expiry-table-body");
            tbody.innerHTML = "";
            for (var i = 0; i < items.length; i++) {
                var e = items[i];
                var tr = document.createElement("tr");
                var badgeClass = "badge-green";
                if (e.status === "Expired") {
                    badgeClass = "badge-red";
                } else if (e.status === "Expiring Soon") {
                    badgeClass = "badge-orange";
                }
                var daysText = e.daysLeft < 0 ? (Math.abs(e.daysLeft) + " days ago") : (e.daysLeft + " days");
                tr.innerHTML = "<td><strong>" + e.sku + "</strong></td>" +
                               "<td>" + e.productName + "</td>" +
                               "<td>" + e.quantity + "</td>" +
                               "<td>" + formatDate(e.expiryDate) + "</td>" +
                               "<td>" + daysText + "</td>" +
                               "<td><span class='badge " + badgeClass + "'>" + e.status + "</span></td>";
                tbody.appendChild(tr);
            }
        });
}

function makeReorderHandler(sku, suggestedQty) {
    return function() {
        document.getElementById("rcv-sku").value = sku;
        document.getElementById("rcv-qty").value = suggestedQty > 0 ? suggestedQty : "";
        document.getElementById("receive-stock-card").scrollIntoView({ behavior: "smooth" });
        document.getElementById(suggestedQty > 0 ? "rcv-exp" : "rcv-qty").focus();
    };
}

function handleReceiveStock(event) {
    event.preventDefault();
    var payload = {
        sku: document.getElementById("rcv-sku").value.trim().toUpperCase(),
        quantity: parseInt(document.getElementById("rcv-qty").value),
        expiryDate: parseInt(document.getElementById("rcv-exp").value.replace(/-/g, ""))
    };
    fetch("/api/batches", {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(payload)
    })
    .then(function(res) { return res.json(); })
    .then(function(data) {
        if (data.success) {
            showNotification(data.message, false);
            document.getElementById("receive-stock-form").reset();
            loadReorder();
        } else {
            showNotification(data.message, true);
        }
    })
    .catch(function(err) {
        showNotification("Failed to connect to server.", true);
    });
}

function loadReorder() {
    fetch("/api/reorder")
        .then(function(res) { return res.json(); })
        .then(function(items) {
            var tbody = document.getElementById("reorder-table-body");
            tbody.innerHTML = "";
            for (var i = 0; i < items.length; i++) {
                var r = items[i];
                var tr = document.createElement("tr");
                var statusBadge = r.needsReorder
                    ? "<span class='badge badge-red'>REORDER NEEDED</span>"
                    : "<span class='badge badge-green'>HEALTHY</span>";
                tr.innerHTML = "<td><strong>" + r.sku + "</strong></td>" +
                               "<td>" + r.productName + "</td>" +
                               "<td>" + r.currentStock + "</td>" +
                               "<td>" + r.leadTime + " days</td>" +
                               "<td>" + r.safetyStock + " units</td>" +
                               "<td>" + r.reorderPoint.toFixed(1) + "</td>" +
                               "<td>" + r.targetStock + "</td>" +
                               "<td><strong>" + r.suggestedQuantity + "</strong>" + (r.limitedByShelf ? " <small style='color:#9a3412;'>(limited by shelf space)</small>" : "") + "</td>" +
                               "<td>" + statusBadge + "</td>";
                var actionCell = document.createElement("td");
                var reorderBtn = document.createElement("button");
                reorderBtn.className = "btn-primary btn-small";
                reorderBtn.textContent = "Reorder";
                reorderBtn.onclick = makeReorderHandler(r.sku, r.suggestedQuantity);
                actionCell.appendChild(reorderBtn);
                tr.appendChild(actionCell);
                tbody.appendChild(tr);
            }
        });
}

function loadProfit() {
    fetch("/api/profit")
        .then(function(res) { return res.json(); })
        .then(function(data) {
            document.getElementById("profit-grand-total").innerText = "₹" + data.grandTotalProfit.toFixed(2);
            document.getElementById("profit-avg-margin").innerText = data.averageMargin.toFixed(1) + "%";
            var tbody = document.getElementById("profit-table-body");
            tbody.innerHTML = "";
            var items = data.items || [];
            for (var i = 0; i < items.length; i++) {
                var p = items[i];
                var tr = document.createElement("tr");
                tr.innerHTML = "<td><strong>" + p.sku + "</strong></td>" +
                               "<td>" + p.productName + "</td>" +
                               "<td>₹" + p.costPrice.toFixed(2) + "</td>" +
                               "<td>₹" + p.sellingPrice.toFixed(2) + "</td>" +
                               "<td>" + p.unitsSold + "</td>" +
                               "<td>₹" + p.profitPerUnit.toFixed(2) + "</td>" +
                               "<td><strong>₹" + p.totalProfit.toFixed(2) + "</strong></td>" +
                               "<td>" + p.profitMarginPercent.toFixed(1) + "%</td>";
                tbody.appendChild(tr);
            }
        });
}

function saveAllData() {
    fetch("/api/save", { method: "POST" })
        .then(function(res) { return res.json(); })
        .then(function(data) {
            showNotification(data.message, false);
        })
        .catch(function(err) {
            showNotification("Failed to save data on server.", true);
        });
}

window.onload = function() {
    loadShelfOptions();
    loadDashboard();
};
