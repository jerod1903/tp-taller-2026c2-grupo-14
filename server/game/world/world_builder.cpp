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
#include "common/map/CommunityCenterInterior.h"
#include "common/map/MarketInterior.h"
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

Position building_door(const maps::MapElement& building) {
    maps::MapCell door = maps::doorCell(building);
    return Position{door.x, door.y};
}

// Los portones de los pasajes se cruzan caminando; la mina, sus escaleras y salidas, con la tecla
bool is_passage(const maps::MapElement& trigger) { return trigger.type == maps::ElementType::Door; }

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

std::set<std::string> passage_ids(const maps::MapDefinition& map) {
    std::set<std::string> ids;
    for (const maps::MapTransition& transition: map.transitions) {
        if (is_passage(element_by_id(map, transition.triggerElementId))) {
            ids.insert(transition.triggerElementId);
        }
    }
    return ids;
}

// El terreno decide qué se camina; además, las estructuras y los NPCs tapan sus celdas salvo
// las de los elementos de `passable`, que hay que pisar para viajar. Las puertas que se usan con
// la tecla, como la de la cabaña, también se tapan: se usan desde al lado.
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
            Tile& tile = tiles.at(cell.y * area.width + cell.x);
            tile = Tile(tile.get_type(), false);
        }
    }
    return Grid(area.width, area.height, std::move(tiles));
}

// El primer elemento de ese tipo en el área; lanza std::invalid_argument si no hay ninguno.
const maps::MapElement& element_of_type(const maps::MapDefinition& map, const maps::MapArea& area,
                                        maps::ElementType type) {
    auto element = std::find_if(map.elements.begin(), map.elements.end(),
                                [&area, type](const maps::MapElement& candidate) {
                                    return candidate.areaId == area.id && candidate.type == type;
                                });
    if (element == map.elements.end()) {
        throw std::invalid_argument("al área " + area.id + " le falta un " +
                                    std::string(maps::toString(type)));
    }
    return *element;
}

// La puerta es la primera house_door del área: se aparece ahí al entrar. Las camas van en los tiles
// sleepable del cuarto, que se ocupan por filas de arriba a abajo y de izquierda a derecha.
std::unique_ptr<Cabin> build_cabin(const maps::MapDefinition& map) {
    const maps::MapArea& area = area_of_type(map, maps::AreaType::CabinArea);
    const maps::MapElement& door = element_of_type(map, area, maps::ElementType::HouseDoor);
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
    return std::make_unique<Cabin>(area.id, build_grid(map, area, {door.id}),
                                   Position{door.x, door.y}, std::move(bed_slots));
}

// Un edificio por dentro (el mercado, el centro comunitario), del area de ese tipo. Lo que hay
// adentro (los comerciantes, los objetos de los sets) tapa su celda.
std::unique_ptr<Interior> build_interior(const maps::MapDefinition& map, maps::AreaType type) {
    const maps::MapArea& area = area_of_type(map, type);
    const maps::MapElement& door = element_of_type(map, area, maps::ElementType::HouseDoor);
    return std::make_unique<Interior>(area.id, build_grid(map, area, {door.id}),
                                      Position{door.x, door.y});
}

// El primer animal_spawn de la granja (donde arrancan los animales); sin uno, la puerta de la
// cabaña.
Position spawn_of(const maps::MapDefinition& map, const maps::MapArea& farm_area,
                  const Farm& farm) {
    auto spawn = std::find_if(map.elements.begin(), map.elements.end(),
                              [&farm_area](const maps::MapElement& element) {
                                  return element.areaId == farm_area.id &&
                                         element.type == maps::ElementType::AnimalSpawn;
                              });
    return spawn != map.elements.end() ? Position{spawn->x, spawn->y} : farm.get_cabin_pos();
}

std::unique_ptr<Farm> build_farm(const maps::MapDefinition& map, const maps::MapArea& area,
                                 const std::set<std::string>& passages,
                                 std::unique_ptr<Cabin> cabin_interior) {
    const maps::MapElement& cabin = element_of_type(map, area, maps::ElementType::Cabin);
    return std::make_unique<Farm>(area.id, build_grid(map, area, passages), building_door(cabin),
                                  std::move(cabin_interior));
}

std::unique_ptr<Town> build_town(const maps::MapDefinition& map, const maps::MapArea& area,
                                 const std::set<std::string>& passages,
                                 std::unique_ptr<Interior> market_interior,
                                 std::unique_ptr<Interior> community_center_interior) {
    const maps::MapElement& market = element_of_type(map, area, maps::ElementType::Market);
    const maps::MapElement& community_center =
            element_of_type(map, area, maps::ElementType::CommunityCenter);
    return std::make_unique<Town>(area.id, build_grid(map, area, passages), building_door(market),
                                  std::move(market_interior), building_door(community_center),
                                  std::move(community_center_interior));
}

// Los niveles se ordenan por su número: el nivel 1 es la entrada, sin importar el orden del
// archivo.
Dungeon build_dungeon(const maps::MapDefinition& map, const std::set<std::string>& passages,
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
                level_area->id, build_grid(map, *level_area, passages), depth));
        locations[level_area->id] = levels.back().get();
    }
    return Dungeon(std::move(levels));
}

// Cada transición es un portal en cada celda del elemento que la activa.
void add_portals(const maps::MapDefinition& map, const LocationsById& locations) {
    for (const maps::MapTransition& transition: map.transitions) {
        Location* from = location_by_id(locations, transition.fromAreaId);
        Location* to = location_by_id(locations, transition.toAreaId);
        Point arrival = center_of(Position{transition.targetX, transition.targetY});
        const maps::MapElement& trigger = element_by_id(map, transition.triggerElementId);
        PortalTrigger kind = is_passage(trigger) ? PortalTrigger::Step : PortalTrigger::Action;
        for (const Position& cell: cells_of(trigger)) {
            from->add_portal(Portal{cell, to, arrival, kind});
        }
    }
}

// Los interiores se leen una sola vez: son los mismos para todas las partidas.
const maps::MapDefinition& cabin_interior() {
    static const maps::MapDefinition interior =
            maps::loadCabinInterior(server_config().game.max_players);
    return interior;
}

const maps::MapDefinition& market_interior() {
    static const maps::MapDefinition interior = maps::loadMarketInterior();
    return interior;
}

const maps::MapDefinition& community_center_interior() {
    static const maps::MapDefinition interior = maps::loadCommunityCenterInterior();
    return interior;
}

const std::map<maps::ElementType, std::string> MERCHANT_NAMES = {
        {maps::ElementType::MerchantMarnie, "Marnie"},
        {maps::ElementType::MerchantClint, "Clint"},
        {maps::ElementType::MerchantPierre, "Pierre"},
        {maps::ElementType::MerchantRobin, "Robin"}};

// Lo que vende y compra cada uno. Clint y Robin todavía no tienen nada: faltan los minerales y
// los muebles. Las semillas de Pierre llegan con los cultivos.
Merchant build_merchant(maps::ElementType type, const Position& pos) {
    const ServerConfig::Market& prices = server_config().market;
    const std::string& name = MERCHANT_NAMES.at(type);
    if (type == maps::ElementType::MerchantMarnie) {
        return Merchant(name, pos,
                        {{"vaca", AnimalKind::Cow, prices.cow_price},
                         {"gallina", AnimalKind::Chicken, prices.chicken_price}},
                        {{"vaca", AnimalKind::Cow, prices.cow_buyback},
                         {"gallina", AnimalKind::Chicken, prices.chicken_buyback}});
    }
    if (type == maps::ElementType::MerchantPierre) {
        return Merchant(name, pos, {}, {{"leche", ItemType::Milk, prices.milk_buyback}});
    }
    return Merchant(name, pos, {}, {});
}

void reject_interior_id(const maps::MapDefinition& map, const std::string& interior_id) {
    for (const maps::MapArea& area: map.areas) {
        if (area.id == interior_id) {
            throw std::invalid_argument("el mapa usa el id de un interior: " + interior_id);
        }
    }
}

}  // namespace

std::unique_ptr<World> build_world(const maps::MapDefinition& map) {
    const maps::MapArea& farm_area = area_of_type(map, maps::AreaType::Farm);
    const maps::MapArea& town_area = area_of_type(map, maps::AreaType::Village);
    std::unique_ptr<Cabin> cabin = build_cabin(cabin_interior());
    std::unique_ptr<Interior> market =
            build_interior(market_interior(), maps::AreaType::MarketArea);
    std::unique_ptr<Interior> community_center =
            build_interior(community_center_interior(), maps::AreaType::CommunityCenterArea);
    reject_interior_id(map, cabin->get_name());
    reject_interior_id(map, market->get_name());
    reject_interior_id(map, community_center->get_name());
    std::set<std::string> passages = passage_ids(map);

    std::unique_ptr<Farm> farm = build_farm(map, farm_area, passages, std::move(cabin));
    std::unique_ptr<Town> town =
            build_town(map, town_area, passages, std::move(market), std::move(community_center));
    LocationsById locations{{farm_area.id, farm.get()}, {town_area.id, town.get()}};
    Dungeon dungeon = build_dungeon(map, passages, locations);

    // Los portales van al final: apuntan a Locations que tienen que existir.
    add_portals(map, locations);
    Position spawn = spawn_of(map, farm_area, *farm);
    // Provisorio, hasta que se compren a Marnie: en fila, un poco mas abajo del animal_spawn (o ahi
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

Market build_market(const maps::MapDefinition& map, World& world) {
    std::vector<Merchant> merchants;
    for (const maps::MapElement& element: market_interior().elements) {
        if (element.type && MERCHANT_NAMES.count(*element.type) > 0) {
            merchants.push_back(build_merchant(*element.type, Position{element.x, element.y}));
        }
    }
    const maps::MapArea& farm_area = area_of_type(map, maps::AreaType::Farm);
    return Market(world.get_town().get_market(), std::move(merchants), world.get_farm(),
                  spawn_of(map, farm_area, world.get_farm()));
}
