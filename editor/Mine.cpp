#include "editor/Mine.h"

#include <algorithm>
#include <numeric>
#include <optional>
#include <string>
#include <vector>

#include "editor/Elements.h"
#include "editor/MapHelper.h"

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

// La transicion que dispara triggerId (en el area from) y lleva al piso, al lado de su salida
maps::MapTransition toFloor(const maps::MapDefinition& map, const std::string& id,
                            const std::string& from, const std::string& triggerId,
                            const std::string& floor) {
    const maps::MapElement* exit = elementOfType(map, floor, maps::ElementType::Exit);
    const int x = exit ? exit->x + exit->width : 1;
    const int y = exit ? exit->y : 1;
    return {id, from, triggerId, floor, x, y};
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
    // Las salidas llevan frente a la entrada
    const maps::MapElement* entrance = elementById(map, entranceId);
    const std::optional<AreaCell> front = entrance ? inFrontOf(map, *entrance) : std::nullopt;
    const std::string entranceArea = entrance ? entrance->areaId : "";
    // Las transiciones despues de crear todos los pisos: la escalera lleva a la salida del
    // siguiente
    for (int index = 0; index < count; ++index) {
        const std::string id = floorId(numbers[index]);
        if (front) {
            map.transitions.push_back(
                    {id + "-salida-ruta", id, id + "-salida", entranceArea, front->x, front->y});
        }
        if (index + 1 < count) {
            map.transitions.push_back(toFloor(map, id + "-escalera-ruta", id, id + "-escalera",
                                              floorId(numbers[index + 1])));
        }
    }
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
