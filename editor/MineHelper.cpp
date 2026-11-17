#include "editor/MineHelper.h"

#include <algorithm>

#include "editor/MapHelper.h"

namespace maps = stardew::maps;

namespace {
bool isFloor(const maps::MapArea& area) { return area.type == maps::AreaType::DungeonLevel; }
}  // namespace

bool hasDungeonLevels(const maps::MapDefinition& map) {
    return std::any_of(map.areas.begin(), map.areas.end(), isFloor);
}

const maps::MapArea* firstDungeonLevel(const maps::MapDefinition& map) {
    const maps::MapArea* first = nullptr;
    for (const maps::MapArea& area: map.areas) {
        if (isFloor(area) && area.levelNumber &&
            (!first || *area.levelNumber < *first->levelNumber)) {
            first = &area;
        }
    }
    return first;
}

std::vector<const maps::MapElement*> dungeonEntrances(const maps::MapDefinition& map) {
    std::vector<const maps::MapElement*> entrances;
    for (const maps::MapElement& element: map.elements) {
        if (element.type == maps::ElementType::DungeonEntrance) {
            entrances.push_back(&element);
        }
    }
    return entrances;
}

std::vector<std::string> areasMovingWith(const maps::MapDefinition& map,
                                         const std::string& areaId) {
    const maps::MapArea* area = areaById(map, areaId);
    if (!area) {
        return {};
    }
    if (!isFloor(*area)) {
        return {areaId};
    }
    std::vector<std::string> floors;
    for (const maps::MapArea& other: map.areas) {
        if (isFloor(other)) {
            floors.push_back(other.id);
        }
    }
    return floors;
}
