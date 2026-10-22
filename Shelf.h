
#include <string>
#include "Utils.h"

class Shelf {
    std::string id_, name_, zone_;
    int capacity_ = 0;
    int used_ = 0;  
public:
    Shelf() = default;
    Shelf(std::string id, std::string name, std::string zone, int capacity)
        : id_(std::move(id)), name_(std::move(name)), zone_(std::move(zone)), capacity_(capacity) {}

    const std::string& id() const { return id_; }
    const std::string& name() const { return name_; }
    const std::string& zone() const { return zone_; }
    int capacity() const { return capacity_; }
    int used() const { return used_; }
    int freeSpace() const { return capacity_ - used_; }
    double usagePct() const { return capacity_ > 0 ? 100.0 * used_ / capacity_ : 0.0; }
    bool canFit(int units) const { return units <= freeSpace(); }

    void setName(const std::string& v) { name_ = v; }
    void setZone(const std::string& v) { zone_ = v; }
    void setCapacity(int v) { capacity_ = v; }
    void setUsed(int v) { used_ = v; }

    json toJson() const {
        return {{"id", id_}, {"name", name_}, {"zone", zone_}, {"capacity", capacity_}};
    }
    static Shelf fromJson(const json& j) {
        return Shelf(j.at("id"), j.at("name"), j.value("zone", "General"), j.at("capacity"));
    }
};
