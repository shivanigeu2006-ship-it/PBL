
#pragma once
#include <map>
#include <vector>
#include <algorithm>
#include <cctype>
#include "Shelf.h"
#include "Inventory.h"

struct ShelfSuggestion {
    std::string type;  // "assign" (unassigned product) or "move" (relieve a crowded shelf)
    std::string sku, name, fromShelf, toShelf, reason;
    int units = 0;
};
NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE(ShelfSuggestion, type, sku, name, fromShelf, toShelf, reason, units)

class ShelfManager {
    std::map<std::string, Shelf> shelves_;

    static std::string lower(std::string s) {
        for (auto& c : s) c = (char)std::tolower((unsigned char)c);
        return s;
    }

    // 2 = zone matches product category, 1 = general-purpose shelf, 0 = unrelated zone
    int zoneRank(const Shelf& s, const Product& p) const {
        if (lower(s.zone()) == lower(p.category())) return 2;
        return lower(s.zone()) == "general" ? 1 : 0;
    }

public:
    bool add(const Shelf& s) { return shelves_.emplace(s.id(), s).second; }
    bool remove(const std::string& id) { return shelves_.erase(id) > 0; }
    void clear() { shelves_.clear(); }
    Shelf* get(const std::string& id) {
        auto it = shelves_.find(id);
        return it == shelves_.end() ? nullptr : &it->second;
    }
    const Shelf* get(const std::string& id) const {
        auto it = shelves_.find(id);
        return it == shelves_.end() ? nullptr : &it->second;
    }
    std::vector<Shelf> all() const {
        std::vector<Shelf> v;
        for (auto& kv : shelves_) v.push_back(kv.second);
        return v;
    }

    // Recalculate used space of every shelf from the stock assigned to it.
    void recompute(const Inventory& inv) {
        for (auto& kv : shelves_) kv.second.setUsed(0);
        for (auto* p : inv.all())
            if (Shelf* s = get(p->shelfId())) s->setUsed(s->used() + p->totalStock());
    }

    // Free space on a shelf from the point of view of one product
    // (its own stock does not count against it if it is already there). -1 if no such shelf.
    int freeFor(const Product& p, const std::string& shelfId) const {
        const Shelf* s = get(shelfId);
        if (!s) return -1;
        return s->freeSpace() + (p.shelfId() == shelfId ? p.totalStock() : 0);
    }

    // Assign (or move) a product to a shelf, rejecting it if the stock does not fit.
    bool assign(Inventory& inv, const std::string& sku, const std::string& shelfId, std::string& err) {
        Product* p = inv.find(sku);
        if (!p) { err = "Unknown SKU: " + sku; return false; }
        if (!get(shelfId)) { err = "Unknown shelf: " + shelfId; return false; }
        int free = freeFor(*p, shelfId);
        if (p->totalStock() > free) {
            err = "Shelf " + shelfId + " has only " + std::to_string(free) + " free space; " +
                  p->name() + " needs " + std::to_string(p->totalStock());
            return false;
        }
        p->setShelf(shelfId);
        recompute(inv);
        return true;
    }

   
    std::string autoAssign(const Product& p, int needed) const {
        std::string best;
        int bestFree = -1, bestRank = -1;
        for (auto& kv : shelves_) {
            int f = freeFor(p, kv.first);
            if (f < needed || f <= 0) continue;
            int rank = zoneRank(kv.second, p);
            if (rank > bestRank || (rank == bestRank && f > bestFree)) {
                best = kv.first; bestFree = f; bestRank = rank;
            }
        }
        return best;
    }

    std::vector<ShelfSuggestion> suggestions(const Inventory& inv) const {
        std::vector<ShelfSuggestion> out;
        std::map<std::string, int> freeMap;
        for (auto& kv : shelves_) freeMap[kv.first] = kv.second.freeSpace();

        auto bestTarget = [&](const Product& p, const std::string& exclude) {
            std::string best;
            int bestFree = -1, bestRank = 0;  // suggestions only use matching or general shelves
            for (auto& kv : shelves_) {
                if (kv.first == exclude) continue;
                int f = freeMap[kv.first];
                if (f < p.totalStock() || f <= 0) continue;
                int rank = zoneRank(kv.second, p);
                if (rank == 0) continue;
                if (rank > bestRank || (rank == bestRank && f > bestFree)) {
                    best = kv.first; bestFree = f; bestRank = rank;
                }
            }
            return best;
        };

        for (auto* p : inv.all()) {
            if (!p->shelfId().empty() && get(p->shelfId())) continue;
            std::string to = bestTarget(*p, "");
            if (to.empty()) continue;
            freeMap[to] -= p->totalStock();
            out.push_back({"assign", p->sku(), p->name(), "", to, "Not on any shelf yet", p->totalStock()});
        }

        for (auto& kv : shelves_) {
            const Shelf& s = kv.second;
            if (s.usagePct() < 85.0) continue;
            int toFree = s.used() - (int)(s.capacity() * 0.70);
            std::vector<const Product*> onShelf;
            for (auto* p : inv.all()) if (p->shelfId() == s.id()) onShelf.push_back(p);
            std::sort(onShelf.begin(), onShelf.end(),
                      [](const Product* a, const Product* b) { return a->totalStock() > b->totalStock(); });
            for (auto* p : onShelf) {
                if (toFree <= 0) break;
                std::string to = bestTarget(*p, s.id());
                if (to.empty()) continue;
                freeMap[to] -= p->totalStock();
                freeMap[s.id()] += p->totalStock();
                toFree -= p->totalStock();
                char buf[96];
                std::snprintf(buf, sizeof buf, "%s is %.0f%% full", s.name().c_str(), s.usagePct());
                out.push_back({"move", p->sku(), p->name(), s.id(), to, buf, p->totalStock()});
            }
        }
        return out;
    }
};
