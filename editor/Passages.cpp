#include "editor/Passages.h"

#include <algorithm>
#include <array>
#include <optional>
#include <string>

namespace maps = stardew::maps;

namespace {

struct Step {
    int dx;
    int dy;
};

constexpr std::array<Step, 4> NEIGHBOURS = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

// Una casilla del mapa general traducida a su area
struct Cell {
    maps::MapArea* area;
    int x;
    int y;
};

std::optional<Cell> cellAt(maps::MapDefinition& map, int x, int y) {
    for (maps::MapArea& area: map.areas) {
        const int localX = x - area.offsetX;
        const int localY = y - area.offsetY;
        if (localX >= 0 && localY >= 0 && localX < area.width && localY < area.height) {
            return Cell{&area, localX, localY};
        }
    }
    return std::nullopt;
}

maps::MapElement* elementAt(maps::MapDefinition& map, const Cell& cell) {
    auto element = std::find_if(map.elements.begin(), map.elements.end(), [&cell](const auto& e) {
        return e.areaId == cell.area->id && cell.x >= e.x && cell.x < e.x + e.width &&
               cell.y >= e.y && cell.y < e.y + e.height;
    });
    return element == map.elements.end() ? nullptr : &*element;
}

bool walkable(const Cell& cell) {
    return cell.y < static_cast<int>(cell.area->tiles.size()) &&
           cell.x < static_cast<int>(cell.area->tiles[cell.y].size()) &&
           maps::isWalkableTerrain(cell.area->tiles[cell.y][cell.x]);
}

// La casilla vecina en otra area y para que lado esta
struct Border {
    Cell there;
    Step step;
};

std::optional<Border> otherSide(maps::MapDefinition& map, const Cell& here, int x, int y) {
    for (const Step& step: NEIGHBOURS) {
        std::optional<Cell> there = cellAt(map, x + step.dx, y + step.dy);
        if (there && there->area != here.area) {
            return Border{*there, step};
        }
    }
    return std::nullopt;
}

// La casilla de al lado dentro de la misma area, o nada si se sale
std::optional<Cell> inside(const Cell& cell, int dx, int dy) {
    const int x = cell.x + dx;
    const int y = cell.y + dy;
    if (x < 0 || y < 0 || x >= cell.area->width || y >= cell.area->height) {
        return std::nullopt;
    }
    return Cell{cell.area, x, y};
}

bool isPassageDoor(const maps::MapElement* element) {
    return element && element->type == maps::ElementType::Door;
}

void removeDoor(maps::MapDefinition& map, const std::string& id) {
    std::erase_if(map.elements, [&id](const auto& e) { return e.id == id; });
    std::erase_if(map.transitions, [&id](const auto& t) { return t.triggerElementId == id; });
}

bool idTaken(const maps::MapDefinition& map, const std::string& id) {
    return std::any_of(map.elements.begin(), map.elements.end(),
                       [&id](const auto& e) { return e.id == id; }) ||
           std::any_of(map.transitions.begin(), map.transitions.end(),
                       [&id](const auto& t) { return t.id == id; });
}

// "granja-pasaje-1", "granja-pasaje-2"... el primero libre, tambien para su transicion
std::string newDoorId(const maps::MapDefinition& map, const maps::MapArea& area) {
    for (int number = 1;; ++number) {
        std::string id = area.id + "-pasaje-" + std::to_string(number);
        if (!idTaken(map, id) && !idTaken(map, id + "-ruta")) {
            return id;
        }
    }
}

void addDoor(maps::MapDefinition& map, const Cell& door, const Cell& arrival) {
    const std::string id = newDoorId(map, *door.area);
    map.elements.push_back({id, door.area->id, maps::ElementType::Door,
                            maps::ElementLayer::Structure, door.x, door.y, 1, 1});
    map.transitions.push_back(
            {id + "-ruta", door.area->id, id, arrival.area->id, arrival.x, arrival.y});
}

}  // namespace

bool togglePassage(maps::MapDefinition& map, int x, int y) {
    const std::optional<Cell> here = cellAt(map, x, y);
    const std::optional<Border> border = here ? otherSide(map, *here, x, y) : std::nullopt;
    if (!border) {
        return false;
    }
    const Cell& there = border->there;
    const Step& step = border->step;
    maps::MapElement* hereElement = elementAt(map, *here);
    maps::MapElement* thereElement = elementAt(map, there);

    // Quitar: un porton a cada lado del borde
    if (isPassageDoor(hereElement) && isPassageDoor(thereElement)) {
        const std::string hereId = hereElement->id;
        const std::string thereId = thereElement->id;
        removeDoor(map, hereId);
        removeDoor(map, thereId);
        return true;
    }

    // Crear: las dos casillas del borde libres y pisables, y lo mismo la de llegada de cada lado
    const std::optional<Cell> hereArrival = inside(*here, -step.dx, -step.dy);
    const std::optional<Cell> thereArrival = inside(there, step.dx, step.dy);
    if (hereElement || thereElement || !hereArrival || !thereArrival || !walkable(*here) ||
        !walkable(there) || !walkable(*hereArrival) || !walkable(*thereArrival)) {
        return false;
    }
    addDoor(map, *here, *thereArrival);
    addDoor(map, there, *hereArrival);
    return true;
}
