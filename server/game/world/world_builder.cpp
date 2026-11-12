#include "server/game/world/world_builder.h"

#include <algorithm>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "server/consts.h"
#include "server/game/world/dungeon/dungeon.h"
#include "server/game/world/farm/cabin.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/grid.h"
#include "server/game/world/tile.h"
#include "server/game/world/town/town.h"

namespace maps = stardew::maps;

namespace {

using LocationsById = std::map<std::string, Location*>;

std::vector<Position> cells_of(const maps::MapElement& element) {
    std::vector<Position> cells;
    for (int y = element.y; y < element.y + element.height; ++y) {
        for (int x = element.x; x < element.x + element.width; ++x) {
            cells.push_back(Position{x, y});
        }
    }
    return cells;
}

Position cabin_door(const maps::MapElement& cabin) {
    maps::MapCell door = maps::doorCell(cabin);
    return Position{door.x, door.y};
}

bool is_trigger(const maps::MapDefinition& map, const maps::MapElement& element) {
    return std::any_of(map.transitions.begin(), map.transitions.end(),
                       [&element](const maps::MapTransition& transition) {
                           return transition.triggerElementId == element.id;
                       });
}

const maps::MapArea& area_of_type(const maps::MapDefinition& map, maps::AreaType type) {
    auto area = std::find_if(map.areas.begin(), map.areas.end(),
                             [type](const maps::MapArea& a) { return a.type == type; });
    if (area == map.areas.end()) {
        throw std::invalid_argument("al mapa le falta un área de tipo " +
                                    std::string(maps::toString(type)));
    }
    return *area;
}

const maps::MapElement& element_by_id(const maps::MapDefinition& map, const std::string& id) {
    auto element = std::find_if(map.elements.begin(), map.elements.end(),
                                [&id](const maps::MapElement& e) { return e.id == id; });
    if (element == map.elements.end()) {
        throw std::invalid_argument("el mapa referencia un elemento inexistente: " + id);
    }
    return *element;
}

Location* location_by_id(const LocationsById& locations, const std::string& id) {
    auto location = locations.find(id);
    if (location == locations.end()) {
        throw std::invalid_argument("el mapa referencia un área inexistente: " + id);
    }
    return location->second;
}

// El terreno decide qué se camina; además, las estructuras y los NPCs tapan sus celdas salvo
// las que hay que pisar para viajar.
Grid build_grid(const maps::MapDefinition& map, const maps::MapArea& area) {
    std::vector<Tile> tiles;
    for (const std::vector<maps::TileType>& row: area.tiles) {
        for (maps::TileType type: row) {
            tiles.emplace_back(type, is_walkable_terrain(type));
        }
    }
    for (const maps::MapElement& element: map.elements) {
        if (element.areaId != area.id || !maps::isSolid(element) || is_trigger(map, element)) {
            continue;
        }
        for (const Position& cell: cells_of(element)) {
            bool is_door = element.type == maps::ElementType::Cabin && cell == cabin_door(element);
            if (!is_door) {
                Tile& tile = tiles.at(cell.y * area.width + cell.x);
                tile = Tile(tile.get_type(), false);
            }
        }
    }
    return Grid(area.width, area.height, std::move(tiles));
}

Grid cabin_interior() {
    std::vector<Tile> tiles(CabinInterior::WIDTH * CabinInterior::HEIGHT,
                            Tile(maps::TileType::WoodFloor, true));
    return Grid(CabinInterior::WIDTH, CabinInterior::HEIGHT, std::move(tiles));
}

// El primer spawn_point de la granja; sin uno, el jugador aparece en la puerta de la cabaña.
Position spawn_of(const maps::MapDefinition& map, const maps::MapArea& farm_area,
                  const Farm& farm) {
    auto spawn = std::find_if(map.elements.begin(), map.elements.end(),
                              [&farm_area](const maps::MapElement& element) {
                                  return element.areaId == farm_area.id &&
                                         element.type == maps::ElementType::SpawnPoint;
                              });
    return spawn != map.elements.end() ? Position{spawn->x, spawn->y} : farm.get_cabin_pos();
}

std::unique_ptr<Farm> build_farm(const maps::MapDefinition& map, const maps::MapArea& area) {
    auto cabin = std::find_if(
            map.elements.begin(), map.elements.end(), [&area](const maps::MapElement& element) {
                return element.areaId == area.id && element.type == maps::ElementType::Cabin;
            });
    if (cabin == map.elements.end()) {
        throw std::invalid_argument("la granja no tiene cabaña");
    }
    return std::make_unique<Farm>(
            area.id, build_grid(map, area), cabin_door(*cabin),
            std::make_unique<Cabin>("cabin", cabin_interior(), CabinInterior::DOOR));
}

// Los pisos se ordenan por su número: el piso 1 es la entrada, sin importar el orden del archivo.
Dungeon build_dungeon(const maps::MapDefinition& map, LocationsById& locations) {
    std::vector<const maps::MapArea*> floors;
    for (const maps::MapArea& area: map.areas) {
        if (area.type == maps::AreaType::DungeonFloor) {
            floors.push_back(&area);
        }
    }
    std::sort(floors.begin(), floors.end(), [](const maps::MapArea* a, const maps::MapArea* b) {
        return a->floorNumber.value_or(0) < b->floorNumber.value_or(0);
    });
    std::vector<std::unique_ptr<DungeonLevel>> levels;
    for (const maps::MapArea* floor: floors) {
        int depth = static_cast<int>(levels.size());
        levels.push_back(std::make_unique<DungeonLevel>(floor->id, build_grid(map, *floor), depth));
        locations[floor->id] = levels.back().get();
    }
    return Dungeon(std::move(levels));
}

// Cada transición es un portal en cada celda del elemento que la activa.
void add_portals(const maps::MapDefinition& map, const LocationsById& locations) {
    for (const maps::MapTransition& transition: map.transitions) {
        Location* from = location_by_id(locations, transition.fromAreaId);
        Location* to = location_by_id(locations, transition.toAreaId);
        Position arrival{transition.targetX, transition.targetY};
        for (const Position& cell: cells_of(element_by_id(map, transition.triggerElementId))) {
            from->add_portal(Portal{cell, to, arrival});
        }
    }
}

}  // namespace

std::unique_ptr<World> build_world(const maps::MapDefinition& map) {
    const maps::MapArea& farm_area = area_of_type(map, maps::AreaType::Farm);
    const maps::MapArea& town_area = area_of_type(map, maps::AreaType::Village);

    std::unique_ptr<Farm> farm = build_farm(map, farm_area);
    std::unique_ptr<Town> town = std::make_unique<Town>(town_area.id, build_grid(map, town_area));
    LocationsById locations{{farm_area.id, farm.get()}, {town_area.id, town.get()}};
    Dungeon dungeon = build_dungeon(map, locations);

    // Los portales van al final: apuntan a Locations que tienen que existir.
    add_portals(map, locations);
    Position spawn = spawn_of(map, farm_area, *farm);
    // Provisorio, hasta que se compren a Marnie: en fila, un poco mas abajo de donde aparece el
    // jugador (o ahi mismo si esa celda no se puede pisar).
    int next = 0;
    auto add_animal = [&](AnimalKind kind) {
        Position pos{spawn.x + ServerConstants::ANIMALS_OFFSET_X + next++,
                     spawn.y + ServerConstants::ANIMALS_OFFSET_Y};
        farm->add_animal(kind, farm->is_walkable(pos) ? pos : spawn);
    };
    for (int i = 0; i < ServerConstants::INITIAL_COWS; ++i) {
        add_animal(AnimalKind::Cow);
    }
    for (int i = 0; i < ServerConstants::INITIAL_CHICKENS; ++i) {
        add_animal(AnimalKind::Chicken);
    }
    return std::make_unique<World>(std::move(farm), std::move(town), std::move(dungeon), spawn);
}
