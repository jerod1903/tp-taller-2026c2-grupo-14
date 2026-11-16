#include "server/game/world/world_builder.h"

#include <algorithm>
#include <map>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/map/CabinInterior.h"
#include "server/config/server_config.h"
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

std::set<std::string> trigger_ids(const maps::MapDefinition& map) {
    std::set<std::string> ids;
    for (const maps::MapTransition& transition: map.transitions) {
        ids.insert(transition.triggerElementId);
    }
    return ids;
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
// las de los elementos de `passable`, que hay que pisar para viajar.
Grid build_grid(const maps::MapDefinition& map, const maps::MapArea& area,
                const std::set<std::string>& passable) {
    std::vector<Tile> tiles;
    for (const std::vector<maps::TileType>& row: area.tiles) {
        for (maps::TileType type: row) {
            tiles.emplace_back(type, is_walkable_terrain(type));
        }
    }
    for (const maps::MapElement& element: map.elements) {
        if (element.areaId != area.id || !maps::isSolid(element) || passable.contains(element.id)) {
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

// La puerta es la primera house_door del área: se aparece ahí al entrar. Las camas van en los tiles
// sleepable del cuarto, que se ocupan por filas de arriba a abajo y de izquierda a derecha.
std::unique_ptr<Cabin> build_cabin(const maps::MapDefinition& map) {
    const maps::MapArea& area = area_of_type(map, maps::AreaType::CabinArea);
    auto door = std::find_if(
            map.elements.begin(), map.elements.end(), [&area](const maps::MapElement& element) {
                return element.areaId == area.id && element.type == maps::ElementType::HouseDoor;
            });
    if (door == map.elements.end()) {
        throw std::invalid_argument("la cabaña no tiene puerta");
    }
    std::vector<Position> bed_slots;
    for (int y = 0; y < static_cast<int>(area.tiles.size()); ++y) {
        for (int x = 0; x < static_cast<int>(area.tiles[y].size()); ++x) {
            if (area.tiles[y][x] == maps::TileType::Sleepable) {
                bed_slots.push_back(Position{x, y});
            }
        }
    }
    if (bed_slots.size() != server_config().game.max_players) {
        throw std::invalid_argument("la cabaña tiene " + std::to_string(bed_slots.size()) +
                                    " tiles sleepable y tiene que tener " +
                                    std::to_string(server_config().game.max_players));
    }
    return std::make_unique<Cabin>(area.id, build_grid(map, area, {door->id}),
                                   Position{door->x, door->y}, std::move(bed_slots));
}

// El primer spawn_point de la granja (donde arrancan los animales); sin uno, la puerta de la
// cabaña.
Position spawn_of(const maps::MapDefinition& map, const maps::MapArea& farm_area,
                  const Farm& farm) {
    auto spawn = std::find_if(map.elements.begin(), map.elements.end(),
                              [&farm_area](const maps::MapElement& element) {
                                  return element.areaId == farm_area.id &&
                                         element.type == maps::ElementType::SpawnPoint;
                              });
    return spawn != map.elements.end() ? Position{spawn->x, spawn->y} : farm.get_cabin_pos();
}

std::unique_ptr<Farm> build_farm(const maps::MapDefinition& map, const maps::MapArea& area,
                                 const std::set<std::string>& triggers,
                                 std::unique_ptr<Cabin> cabin_interior) {
    auto cabin = std::find_if(
            map.elements.begin(), map.elements.end(), [&area](const maps::MapElement& element) {
                return element.areaId == area.id && element.type == maps::ElementType::Cabin;
            });
    if (cabin == map.elements.end()) {
        throw std::invalid_argument("la granja no tiene cabaña");
    }
    return std::make_unique<Farm>(area.id, build_grid(map, area, triggers), cabin_door(*cabin),
                                  std::move(cabin_interior));
}

// Los niveles se ordenan por su número: el nivel 1 es la entrada, sin importar el orden del
// archivo.
Dungeon build_dungeon(const maps::MapDefinition& map, const std::set<std::string>& triggers,
                      LocationsById& locations) {
    std::vector<const maps::MapArea*> level_areas;
    for (const maps::MapArea& area: map.areas) {
        if (area.type == maps::AreaType::DungeonLevel) {
            level_areas.push_back(&area);
        }
    }
    std::sort(level_areas.begin(), level_areas.end(),
              [](const maps::MapArea* a, const maps::MapArea* b) {
                  return a->levelNumber.value_or(0) < b->levelNumber.value_or(0);
              });
    std::vector<std::unique_ptr<DungeonLevel>> levels;
    for (const maps::MapArea* level_area: level_areas) {
        int depth = static_cast<int>(levels.size());
        levels.push_back(std::make_unique<DungeonLevel>(
                level_area->id, build_grid(map, *level_area, triggers), depth));
        locations[level_area->id] = levels.back().get();
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

// Se lee una sola vez: es el mismo para todas las partidas.
const maps::MapDefinition& cabin_interior() {
    static const maps::MapDefinition interior =
            maps::loadCabinInterior(server_config().game.max_players);
    return interior;
}

void reject_cabin_id(const maps::MapDefinition& map, const std::string& cabin_id) {
    for (const maps::MapArea& area: map.areas) {
        if (area.id == cabin_id) {
            throw std::invalid_argument("el mapa usa el id del interior de la cabaña: " + cabin_id);
        }
    }
}

}  // namespace

std::unique_ptr<World> build_world(const maps::MapDefinition& map) {
    const maps::MapArea& farm_area = area_of_type(map, maps::AreaType::Farm);
    const maps::MapArea& town_area = area_of_type(map, maps::AreaType::Village);
    std::unique_ptr<Cabin> cabin = build_cabin(cabin_interior());
    reject_cabin_id(map, cabin->get_name());
    std::set<std::string> triggers = trigger_ids(map);

    std::unique_ptr<Farm> farm = build_farm(map, farm_area, triggers, std::move(cabin));
    std::unique_ptr<Town> town =
            std::make_unique<Town>(town_area.id, build_grid(map, town_area, triggers));
    LocationsById locations{{farm_area.id, farm.get()}, {town_area.id, town.get()}};
    Dungeon dungeon = build_dungeon(map, triggers, locations);

    // Los portales van al final: apuntan a Locations que tienen que existir.
    add_portals(map, locations);
    Position spawn = spawn_of(map, farm_area, *farm);
    // Provisorio, hasta que se compren a Marnie: en fila, un poco mas abajo del spawn_point (o ahi
    // mismo si esa celda no se puede pisar).
    int next = 0;
    auto add_animal = [&](AnimalKind kind) {
        Position pos{spawn.x + server_config().animals.offset_x + next++,
                     spawn.y + server_config().animals.offset_y};
        farm->add_animal(kind, farm->is_walkable(pos) ? pos : spawn);
    };
    for (int i = 0; i < server_config().animals.cows; ++i) {
        add_animal(AnimalKind::Cow);
    }
    for (int i = 0; i < server_config().animals.chickens; ++i) {
        add_animal(AnimalKind::Chicken);
    }
    return std::make_unique<World>(std::move(farm), std::move(town), std::move(dungeon));
}
