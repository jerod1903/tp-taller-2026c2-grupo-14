#include "editor/ElementHelper.h"

#include <algorithm>

namespace maps = stardew::maps;

namespace {
// Una casilla libre que se puede pisar
bool freeToStand(const maps::MapDefinition& map, const AreaCell& cell) {
    return walkable(map, cell) && !elementAt(map, cell);
}
}  // namespace

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

const maps::MapElement* elementOfType(const maps::MapDefinition& map, const std::string& areaId,
                                      maps::ElementType type) {
    auto element = std::find_if(map.elements.begin(), map.elements.end(), [&](const auto& e) {
        return e.areaId == areaId && e.type == type;
    });
    return element == map.elements.end() ? nullptr : &*element;
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
                                   int width, int height, const std::string& ignoredId) {
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
            const maps::MapElement* other = elementAt(map, {topLeft->area, x, y});
            if (other && other->id != ignoredId) {
                return std::nullopt;
            }
        }
    }
    return topLeft;
}

maps::MapCell bottomLeftOf(const maps::MapDefinition& map, const maps::MapElement& element) {
    const maps::MapArea* area = areaById(map, element.areaId);
    const int offsetX = area ? area->offsetX : 0;
    const int offsetY = area ? area->offsetY : 0;
    return {offsetX + element.x, offsetY + element.y + element.height - 1};
}

std::optional<AreaCell> inFrontOf(const maps::MapDefinition& map, const maps::MapElement& element) {
    const std::optional<std::size_t> index = areaIndex(map, element.areaId);
    if (!index) {
        return std::nullopt;
    }
    const int x = element.x + element.width / 2;
    for (int y = element.y + element.height; y < map.areas[*index].height; ++y) {
        const AreaCell cell{*index, x, y};
        if (freeToStand(map, cell)) {
            return cell;
        }
    }
    return std::nullopt;
}

std::optional<AreaCell> besideOf(const maps::MapDefinition& map, const maps::MapElement& element) {
    const std::optional<std::size_t> index = areaIndex(map, element.areaId);
    if (!index) {
        return std::nullopt;
    }
    const AreaCell base{*index, element.x, element.y};
    for (const Step& step: NEIGHBOURS) {
        // Hacia la derecha y hacia abajo, el vecino esta despues de todo el ancho o el alto
        const int dx = step.dx > 0 ? element.width : step.dx;
        const int dy = step.dy > 0 ? element.height : step.dy;
        const std::optional<AreaCell> cell = inside(map, base, dx, dy);
        if (cell && freeToStand(map, *cell)) {
            return cell;
        }
    }
    return std::nullopt;
}
