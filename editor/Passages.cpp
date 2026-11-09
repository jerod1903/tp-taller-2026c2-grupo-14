#include "editor/Passages.h"

#include <array>
#include <optional>
#include <string>

#include "editor/Elements.h"
#include "editor/MapHelper.h"

namespace maps = stardew::maps;

namespace {

struct Step {
    int dx;
    int dy;
};

constexpr std::array<Step, 4> NEIGHBOURS = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

// La casilla vecina en otra area y para que lado esta
struct Border {
    AreaCell there;
    Step step;
};

std::optional<Border> otherSide(const maps::MapDefinition& map, const AreaCell& here, int x,
                                int y) {
    for (const Step& step: NEIGHBOURS) {
        std::optional<AreaCell> there = cellAt(map, x + step.dx, y + step.dy);
        if (there && there->area != here.area) {
            return Border{*there, step};
        }
    }
    return std::nullopt;
}

bool isPassageDoor(const maps::MapElement* element) {
    return element && element->type == maps::ElementType::Door;
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

void addDoor(maps::MapDefinition& map, const AreaCell& door, const AreaCell& arrival) {
    const std::string doorArea = map.areas[door.area].id;
    const std::string arrivalArea = map.areas[arrival.area].id;
    const std::string id = newDoorId(map, map.areas[door.area]);
    map.elements.push_back({id, doorArea, maps::ElementType::Door, maps::ElementLayer::Structure,
                            door.x, door.y, 1, 1});
    map.transitions.push_back({id + "-ruta", doorArea, id, arrivalArea, arrival.x, arrival.y});
}

}  // namespace

bool togglePassage(maps::MapDefinition& map, int x, int y) {
    const std::optional<AreaCell> here = cellAt(map, x, y);
    const std::optional<Border> border = here ? otherSide(map, *here, x, y) : std::nullopt;
    if (!border) {
        return false;
    }
    const AreaCell& there = border->there;
    const Step& step = border->step;
    const maps::MapElement* hereElement = elementAt(map, *here);
    const maps::MapElement* thereElement = elementAt(map, there);

    // Quitar: un porton a cada lado del borde
    if (isPassageDoor(hereElement) && isPassageDoor(thereElement)) {
        const std::string hereId = hereElement->id;
        const std::string thereId = thereElement->id;
        removeElement(map, hereId);
        removeElement(map, thereId);
        return true;
    }

    // Crear: las dos casillas del borde libres y pisables, y lo mismo la de llegada de cada lado
    const std::optional<AreaCell> hereArrival = inside(map, *here, -step.dx, -step.dy);
    const std::optional<AreaCell> thereArrival = inside(map, there, step.dx, step.dy);
    if (hereElement || thereElement || !hereArrival || !thereArrival || !walkable(map, *here) ||
        !walkable(map, there) || !walkable(map, *hereArrival) || !walkable(map, *thereArrival)) {
        return false;
    }
    addDoor(map, *here, *thereArrival);
    addDoor(map, there, *hereArrival);
    return true;
}

bool removePassage(maps::MapDefinition& map, const std::string& doorId) {
    const maps::MapElement* door = elementById(map, doorId);
    const maps::MapArea* area = door ? areaById(map, door->areaId) : nullptr;
    if (!isPassageDoor(door) || !area) {
        return false;
    }
    // El porton del otro lado del borde, si esta
    const int x = area->offsetX + door->x;
    const int y = area->offsetY + door->y;
    const std::optional<AreaCell> here = cellAt(map, x, y);
    const std::optional<Border> border = here ? otherSide(map, *here, x, y) : std::nullopt;
    const maps::MapElement* pair = border ? elementAt(map, border->there) : nullptr;
    const std::string pairId = isPassageDoor(pair) ? pair->id : "";
    removeElement(map, doorId);
    if (!pairId.empty()) {
        removeElement(map, pairId);
    }
    return true;
}
