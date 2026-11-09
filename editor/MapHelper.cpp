#include "editor/MapHelper.h"

#include <algorithm>

namespace maps = stardew::maps;

std::optional<AreaCell> cellAt(const maps::MapDefinition& map, int x, int y) {
    for (std::size_t index = 0; index < map.areas.size(); ++index) {
        const maps::MapArea& area = map.areas[index];
        const int localX = x - area.offsetX;
        const int localY = y - area.offsetY;
        if (localX >= 0 && localY >= 0 && localX < area.width && localY < area.height) {
            return AreaCell{index, localX, localY};
        }
    }
    return std::nullopt;
}

const maps::MapElement* elementAt(const maps::MapDefinition& map, const AreaCell& cell) {
    const std::string& areaId = map.areas[cell.area].id;
    auto element = std::find_if(map.elements.begin(), map.elements.end(), [&](const auto& e) {
        return e.areaId == areaId && cell.x >= e.x && cell.x < e.x + e.width && cell.y >= e.y &&
               cell.y < e.y + e.height;
    });
    return element == map.elements.end() ? nullptr : &*element;
}

bool walkable(const maps::MapDefinition& map, const AreaCell& cell) {
    const auto& tiles = map.areas[cell.area].tiles;
    return cell.y < static_cast<int>(tiles.size()) &&
           cell.x < static_cast<int>(tiles[cell.y].size()) &&
           maps::isWalkableTerrain(tiles[cell.y][cell.x]);
}

std::optional<AreaCell> inside(const maps::MapDefinition& map, const AreaCell& cell, int dx,
                               int dy) {
    const maps::MapArea& area = map.areas[cell.area];
    const int x = cell.x + dx;
    const int y = cell.y + dy;
    if (x < 0 || y < 0 || x >= area.width || y >= area.height) {
        return std::nullopt;
    }
    return AreaCell{cell.area, x, y};
}

bool idTaken(const maps::MapDefinition& map, const std::string& id) {
    return std::any_of(map.elements.begin(), map.elements.end(),
                       [&id](const auto& e) { return e.id == id; }) ||
           std::any_of(map.transitions.begin(), map.transitions.end(),
                       [&id](const auto& t) { return t.id == id; });
}
