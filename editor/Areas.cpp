#include "editor/Areas.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "editor/Passages.h"

namespace maps = stardew::maps;

namespace {
// El nombre de un area nueva: granja o pueblo (los pisos de la mina los crea la entrada)
std::string areaName(maps::AreaType type) {
    return type == maps::AreaType::Farm ? "granja" : "pueblo";
}

// "granja", "granja-2", "granja-3"... el primero libre
std::string newAreaId(const maps::MapDefinition& map, maps::AreaType type) {
    const std::string name = areaName(type);
    if (!areaById(map, name)) {
        return name;
    }
    for (int number = 2;; ++number) {
        std::string id = name + "-" + std::to_string(number);
        if (!areaById(map, id)) {
            return id;
        }
    }
}
}  // namespace

void moveAreas(maps::MapDefinition& map, const std::vector<std::string>& areaIds, int dx, int dy) {
    // Los pasajes se buscan con las areas donde estan: hay que quitarlos antes de correrlas
    for (const std::string& doorId: passagesBrokenBy(map, areaIds, dx, dy)) {
        removePassage(map, doorId);
    }
    for (maps::MapArea& area: map.areas) {
        if (std::find(areaIds.begin(), areaIds.end(), area.id) != areaIds.end()) {
            area.offsetX += dx;
            area.offsetY += dy;
        }
    }
}

std::string addArea(maps::MapDefinition& map, maps::AreaType type, const AreaSize& size,
                    const maps::MapCell& topLeft) {
    maps::MapArea area;
    area.id = newAreaId(map, type);
    area.type = type;
    area.width = size.width;
    area.height = size.height;
    area.offsetX = topLeft.x;
    area.offsetY = topLeft.y;
    area.tiles.assign(size.height, std::vector<maps::TileType>(size.width, maps::TileType::Grass));
    map.areas.push_back(std::move(area));
    return map.areas.back().id;
}

void removeAreas(maps::MapDefinition& map, const std::vector<std::string>& areaIds) {
    auto removed = [&areaIds](const std::string& areaId) {
        return std::find(areaIds.begin(), areaIds.end(), areaId) != areaIds.end();
    };
    // Primero los pasajes, que se llevan el porton de la otra area
    std::vector<std::string> doors;
    for (const maps::MapElement& element: map.elements) {
        if (element.type == maps::ElementType::Door && removed(element.areaId)) {
            doors.push_back(element.id);
        }
    }
    for (const std::string& door: doors) {
        removePassage(map, door);
    }
    std::erase_if(map.elements, [&](const auto& e) { return removed(e.areaId); });
    std::erase_if(map.transitions,
                  [&](const auto& t) { return removed(t.fromAreaId) || removed(t.toAreaId); });
    std::erase_if(map.areas, [&](const auto& a) { return removed(a.id); });
}
