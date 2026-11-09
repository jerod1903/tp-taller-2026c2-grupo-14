#include "editor/Elements.h"

#include <optional>
#include <string>
#include <vector>

#include "editor/MapHelper.h"

namespace maps = stardew::maps;

namespace {

// "market-1", "market-2"... el primero libre
std::string newElementId(const maps::MapDefinition& map, maps::ElementType type) {
    for (int number = 1;; ++number) {
        std::string id = std::string(maps::toString(type)) + "-" + std::to_string(number);
        if (!idTaken(map, id)) {
            return id;
        }
    }
}

}  // namespace

maps::MapElement elementFromRule(const maps::ElementRule& rule, const std::string& id,
                                 const std::string& areaId, int x, int y) {
    return {id, areaId, rule.type, rule.layer, x, y, rule.width, rule.height};
}

bool removeElement(maps::MapDefinition& map, const std::string& id) {
    if (std::erase_if(map.elements, [&id](const auto& e) { return e.id == id; }) == 0) {
        return false;
    }
    std::erase_if(map.transitions, [&id](const auto& t) { return t.triggerElementId == id; });
    return true;
}

std::optional<std::string> placeElement(maps::MapDefinition& map, const maps::ElementRule& rule,
                                        const maps::MapCell& bottomLeft) {
    const std::optional<AreaCell> topLeft = freeBaseAt(map, bottomLeft, rule.width, rule.height);
    if (!topLeft) {
        return std::nullopt;
    }
    std::string id = newElementId(map, rule.type);
    map.elements.push_back(
            elementFromRule(rule, id, map.areas[topLeft->area].id, topLeft->x, topLeft->y));
    return id;
}
