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

const maps::MapArea* areaById(const maps::MapDefinition& map, const std::string& id) {
    auto area = std::find_if(map.areas.begin(), map.areas.end(),
                             [&id](const auto& a) { return a.id == id; });
    return area == map.areas.end() ? nullptr : &*area;
}

const maps::MapElement* elementAt(const maps::MapDefinition& map, const AreaCell& cell) {
    const std::string& areaId = map.areas[cell.area].id;
    auto element = std::find_if(map.elements.begin(), map.elements.end(), [&](const auto& e) {
        return e.areaId == areaId && cell.x >= e.x && cell.x < e.x + e.width && cell.y >= e.y &&
               cell.y < e.y + e.height;
    });
    return element == map.elements.end() ? nullptr : &*element;
}

const maps::MapElement* elementById(const maps::MapDefinition& map, const std::string& id) {
    auto element = std::find_if(map.elements.begin(), map.elements.end(),
                                [&id](const auto& e) { return e.id == id; });
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

const maps::ElementRule* ruleFor(const maps::MapRequirements& requirements,
                                 maps::ElementType type) {
    auto rule = std::find_if(requirements.elements.begin(), requirements.elements.end(),
                             [type](const auto& r) { return r.type == type; });
    return rule == requirements.elements.end() ? nullptr : &*rule;
}

std::optional<AreaCell> freeBaseAt(const maps::MapDefinition& map, const maps::MapCell& bottomLeft,
                                   int width, int height) {
    const std::optional<AreaCell> topLeft = cellAt(map, bottomLeft.x, bottomLeft.y - height + 1);
    if (!topLeft) {
        return std::nullopt;
    }
    const maps::MapArea& area = map.areas[topLeft->area];
    if (topLeft->x + width > area.width || topLeft->y + height > area.height) {
        return std::nullopt;
    }
    for (int y = topLeft->y; y < topLeft->y + height; ++y) {
        for (int x = topLeft->x; x < topLeft->x + width; ++x) {
            if (elementAt(map, {topLeft->area, x, y})) {
                return std::nullopt;
            }
        }
    }
    return topLeft;
}
