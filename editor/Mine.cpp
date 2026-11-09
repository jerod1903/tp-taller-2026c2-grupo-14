#include "editor/Mine.h"

#include <algorithm>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "editor/ElementHelper.h"
#include "editor/Elements.h"
#include "editor/MapHelper.h"
#include "editor/MineHelper.h"

namespace maps = stardew::maps;

namespace {
constexpr int FLOORS_PER_ROW = 5;

// Los numeros de piso que piden las reglas, de menor a mayor
std::vector<int> floorNumbers(const maps::MapRequirements& requirements) {
    auto floors =
            std::find_if(requirements.areas.begin(), requirements.areas.end(),
                         [](const auto& a) { return a.type == maps::AreaType::DungeonFloor; });
    if (floors == requirements.areas.end()) {
        return {};
    }
    std::vector<int> numbers = floors->floorNumbers;
    if (numbers.empty()) {
        numbers.resize(floors->count);
        std::iota(numbers.begin(), numbers.end(), 1);
    }
    std::sort(numbers.begin(), numbers.end());
    return numbers;
}

std::string floorId(int number) { return "mina-" + std::to_string(number); }

// Un piso vacio: pared en el borde y piso adentro. index es su lugar en las filas de pisos, que
// empiezan en la fila top del mapa general.
maps::MapArea floorArea(int number, const FloorSize& size, int index, int top) {
    maps::MapArea area;
    area.id = floorId(number);
    area.type = maps::AreaType::DungeonFloor;
    area.floorNumber = number;
    area.width = size.width;
    area.height = size.height;
    area.offsetX = (index % FLOORS_PER_ROW) * (size.width + AREA_GAP);
    area.offsetY = top + (index / FLOORS_PER_ROW) * (size.height + AREA_GAP);
    area.tiles.assign(size.height,
                      std::vector<maps::TileType>(size.width, maps::TileType::CaveWall));
    for (int y = 1; y < size.height - 1; ++y) {
        std::fill(area.tiles[y].begin() + 1, area.tiles[y].end() - 1, maps::TileType::CaveFloor);
    }
    return area;
}

// La salida o la escalera de un piso, con el tamano y la capa de su regla
void addFloorElement(maps::MapDefinition& map, const maps::MapRequirements& requirements,
                     maps::ElementType type, const std::string& id, const std::string& floor, int x,
                     int y) {
    if (const maps::ElementRule* rule = ruleFor(requirements, type)) {
        map.elements.push_back(elementFromRule(*rule, id, floor, x, y));
    }
}

// La casilla del piso donde aparece quien llega: al lado de su salida
maps::MapCell arrivalIn(const maps::MapDefinition& map, const std::string& floor) {
    const maps::MapElement* exit = elementOfType(map, floor, maps::ElementType::Exit);
    const std::optional<AreaCell> beside = exit ? besideOf(map, *exit) : std::nullopt;
    return beside ? maps::MapCell{beside->x, beside->y} : maps::MapCell{1, 1};
}

// La transicion que dispara triggerId (en el area from) y lleva al piso, al lado de su salida
maps::MapTransition toFloor(const maps::MapDefinition& map, const std::string& id,
                            const std::string& from, const std::string& triggerId,
                            const std::string& floor) {
    const maps::MapCell arrival = arrivalIn(map, floor);
    return {id, from, triggerId, floor, arrival.x, arrival.y};
}

bool isFloor(const maps::MapArea& area) { return area.type == maps::AreaType::DungeonFloor; }

// Donde quedaria, dentro de su piso, la esquina de arriba a la izquierda de un elemento de ese
// tipo apoyado en bottomLeft, y cuanto mide. Vacio si no cae en un piso o no hay ninguno de ese
// tipo.
struct FloorSpot {
    int x;
    int y;
    int width;
    int height;
};

std::optional<FloorSpot> floorSpotOf(const maps::MapDefinition& map, maps::ElementType type,
                                     const maps::MapCell& bottomLeft) {
    auto sample = std::find_if(map.elements.begin(), map.elements.end(),
                               [type](const auto& e) { return e.type == type; });
    if (sample == map.elements.end()) {
        return std::nullopt;
    }
    const std::optional<AreaCell> topLeft =
            cellAt(map, bottomLeft.x, bottomLeft.y - sample->height + 1);
    if (!topLeft || !isFloor(map.areas[topLeft->area])) {
        return std::nullopt;
    }
    return FloorSpot{topLeft->x, topLeft->y, sample->width, sample->height};
}
}  // namespace

void createMine(maps::MapDefinition& map, const maps::MapRequirements& requirements,
                const FloorSize& size, const std::string& entranceId) {
    const std::vector<int> numbers = floorNumbers(requirements);
    const int count = static_cast<int>(numbers.size());
    const int top = bottomOfAreas(map) + AREA_GAP;
    for (int index = 0; index < count; ++index) {
        const std::string id = floorId(numbers[index]);
        map.areas.push_back(floorArea(numbers[index], size, index, top));
        addFloorElement(map, requirements, maps::ElementType::Exit, id + "-salida", id, 1, 1);
        // El ultimo piso no tiene escalera: no hay adonde bajar
        if (index + 1 < count) {
            addFloorElement(map, requirements, maps::ElementType::Stairs, id + "-escalera", id,
                            size.width - 2, size.height - 2);
        }
    }
    // Las transiciones despues de crear todos los pisos: la escalera lleva a la salida del
    // siguiente
    for (int index = 0; index + 1 < count; ++index) {
        const std::string id = floorId(numbers[index]);
        map.transitions.push_back(toFloor(map, id + "-escalera-ruta", id, id + "-escalera",
                                          floorId(numbers[index + 1])));
    }
    linkExits(map, entranceId);
}

void linkExits(maps::MapDefinition& map, const std::string& entranceId) {
    const maps::MapElement* entrance = elementById(map, entranceId);
    const std::optional<AreaCell> front = entrance ? inFrontOf(map, *entrance) : std::nullopt;
    if (!front) {
        return;
    }
    const std::string entranceArea = entrance->areaId;
    for (const maps::MapElement& exit: map.elements) {
        if (exit.type != maps::ElementType::Exit) {
            continue;
        }
        auto transition =
                std::find_if(map.transitions.begin(), map.transitions.end(),
                             [&exit](const auto& t) { return t.triggerElementId == exit.id; });
        if (transition == map.transitions.end()) {
            map.transitions.push_back({exit.id + "-ruta", exit.areaId, exit.id, "", 0, 0});
            transition = map.transitions.end() - 1;
        }
        transition->toAreaId = entranceArea;
        transition->targetX = front->x;
        transition->targetY = front->y;
    }
}

bool fitsInEveryFloor(const maps::MapDefinition& map, maps::ElementType type,
                      const maps::MapCell& bottomLeft) {
    const std::optional<FloorSpot> spot = floorSpotOf(map, type, bottomLeft);
    if (!spot) {
        return false;
    }
    for (const maps::MapArea& floor: map.areas) {
        const maps::MapElement* own = isFloor(floor) ? elementOfType(map, floor.id, type) : nullptr;
        if (!own) {
            continue;
        }
        const maps::MapCell there{floor.offsetX + spot->x,
                                  floor.offsetY + spot->y + spot->height - 1};
        if (!freeBaseAt(map, there, spot->width, spot->height, own->id)) {
            return false;
        }
    }
    return true;
}

bool moveInEveryFloor(maps::MapDefinition& map, maps::ElementType type,
                      const maps::MapCell& bottomLeft) {
    if (!fitsInEveryFloor(map, type, bottomLeft)) {
        return false;
    }
    const std::optional<FloorSpot> spot = floorSpotOf(map, type, bottomLeft);
    for (maps::MapElement& element: map.elements) {
        if (element.type == type) {
            element.x = spot->x;
            element.y = spot->y;
        }
    }
    // Quien llega a un piso aparece al lado de la salida, que pudo haberse movido o quedar tapada
    for (maps::MapTransition& transition: map.transitions) {
        const maps::MapArea* to = areaById(map, transition.toAreaId);
        if (to && isFloor(*to)) {
            const maps::MapCell arrival = arrivalIn(map, to->id);
            transition.targetX = arrival.x;
            transition.targetY = arrival.y;
        }
    }
    return true;
}

void linkEntrance(maps::MapDefinition& map, const std::string& entranceId) {
    const maps::MapElement* entrance = elementById(map, entranceId);
    const maps::MapArea* first = firstDungeonFloor(map);
    if (!entrance || !first) {
        return;
    }
    map.transitions.push_back(
            toFloor(map, entranceId + "-ruta", entrance->areaId, entranceId, first->id));
}
