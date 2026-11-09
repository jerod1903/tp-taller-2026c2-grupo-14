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

std::optional<std::size_t> areaIndex(const maps::MapDefinition& map, const std::string& id) {
    auto area = std::find_if(map.areas.begin(), map.areas.end(),
                             [&id](const auto& a) { return a.id == id; });
    if (area == map.areas.end()) {
        return std::nullopt;
    }
    return static_cast<std::size_t>(area - map.areas.begin());
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

CellRect areaRect(const maps::MapArea& area) {
    return {area.offsetX, area.offsetY, area.width, area.height};
}

bool overlapsAreas(const maps::MapDefinition& map, const std::vector<CellRect>& rects,
                   const std::vector<std::string>& ignoredAreaIds) {
    for (const maps::MapArea& area: map.areas) {
        if (std::find(ignoredAreaIds.begin(), ignoredAreaIds.end(), area.id) !=
            ignoredAreaIds.end()) {
            continue;
        }
        const CellRect other = areaRect(area);
        for (const CellRect& rect: rects) {
            if (rect.x < other.x + other.width && other.x < rect.x + rect.width &&
                rect.y < other.y + other.height && other.y < rect.y + rect.height) {
                return true;
            }
        }
    }
    return false;
}

int bottomOfAreas(const maps::MapDefinition& map) {
    int bottom = 0;
    for (const maps::MapArea& area: map.areas) {
        bottom = std::max(bottom, area.offsetY + area.height);
    }
    return bottom;
}
