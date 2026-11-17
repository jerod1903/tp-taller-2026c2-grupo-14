#include "server/game/world/world_builder.h"

#include <algorithm>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "common/map/persistence/MapRepository.h"
#include "server/config/server_config.h"

namespace {

namespace maps = stardew::maps;
using maps::AreaType;
using maps::ElementLayer;
using maps::ElementType;

maps::MapArea area(const std::string& id, AreaType type, int width, int height,
                   TileType fill = TileType::Grass, std::optional<int> floor = std::nullopt) {
    std::vector<std::vector<TileType>> tiles(height, std::vector<TileType>(width, fill));
    return maps::MapArea{id, type, width, height, floor, tiles};
}

maps::MapElement element(const std::string& id, const std::string& area_id, ElementType type,
                         ElementLayer layer, int x, int y, int width = 1, int height = 1) {
    return maps::MapElement{id, area_id, type, layer, x, y, width, height};
}

std::string floor_id(int floor) { return "dungeon-" + std::to_string(floor); }

// Granja con cabaña de 3x2 en (1,1), pueblo con mercado, centro comunitario de 2x1 en (2,2),
// mercader y agua, y los 10 pisos cargados
// del más profundo al primero. Cada piso tiene la salida en (0,0) y una escalera de 2x1 en (1,0).
maps::MapDefinition make_map() {
    maps::MapDefinition map{1, "Test", {}, {}, {}};
    map.areas.push_back(area("farm", AreaType::Farm, 6, 5));
    map.areas.push_back(area("village", AreaType::Village, 6, 4, TileType::Path));
    map.areas.back().tiles[3][0] = TileType::Water;
    for (int floor = 10; floor >= 1; --floor) {
        map.areas.push_back(
                area(floor_id(floor), AreaType::DungeonLevel, 3, 1, TileType::CaveFloor, floor));
    }
    map.elements = {
            element("cabin", "farm", ElementType::Cabin, ElementLayer::Structure, 1, 1, 3, 2),
            element("farm-gate", "farm", ElementType::Door, ElementLayer::Structure, 5, 4),
            element("animals", "farm", ElementType::AnimalSpawn, ElementLayer::Object, 0, 4),
            element("market", "village", ElementType::Market, ElementLayer::Structure, 0, 0, 2, 1),
            element("center", "village", ElementType::CommunityCenter, ElementLayer::Structure, 2,
                    2, 2, 1),
            element("marnie", "village", ElementType::MerchantMarnie, ElementLayer::Npc, 3, 0),
            element("mine", "village", ElementType::DungeonEntrance, ElementLayer::Structure, 5, 0),
    };
    map.transitions = {{"to-village", "farm", "farm-gate", "village", 4, 3},
                       {"to-mine", "village", "mine", floor_id(1), 0, 0}};
    for (int floor = 1; floor <= 10; ++floor) {
        std::string exit = "exit-" + std::to_string(floor);
        map.elements.push_back(
                element(exit, floor_id(floor), ElementType::Door, ElementLayer::Structure, 0, 0));
        map.transitions.push_back({exit + "-route", floor_id(floor), exit, "village", 5, 1});
        if (floor < 10) {
            std::string stairs = "stairs-" + std::to_string(floor);
            map.elements.push_back(element(stairs, floor_id(floor), ElementType::Stairs,
                                           ElementLayer::Structure, 1, 0, 2, 1));
            map.transitions.push_back(
                    {stairs + "-route", floor_id(floor), stairs, floor_id(floor + 1), 0, 0});
        }
    }
    return map;
}

class WorldBuilderTest: public ::testing::Test {
protected:
    std::unique_ptr<World> world = build_world(make_map());
    Farm& farm() { return world->get_farm(); }
    Town& town() { return world->get_town(); }
    DungeonLevel& level(int depth) { return world->get_dungeon().level(depth); }
};

TEST_F(WorldBuilderTest, AreasAreNamedByTheirId) {
    EXPECT_EQ(farm().get_name(), "farm");
    EXPECT_EQ(town().get_name(), "village");
    EXPECT_EQ(farm().get_grid().get_width(), 6);
}

TEST_F(WorldBuilderTest, DungeonLevelsAreOrderedByFloor) {
    for (int depth = 0; depth < Dungeon::LEVEL_COUNT; ++depth) {
        EXPECT_EQ(level(depth).get_name(), floor_id(depth + 1));
        EXPECT_EQ(level(depth).get_depth(), depth);
    }
}

TEST_F(WorldBuilderTest, TerrainDecidesWhatIsWalkable) {
    EXPECT_FALSE(town().is_walkable(Position{0, 3}));
    EXPECT_TRUE(town().is_walkable(Position{1, 3}));
}

TEST_F(WorldBuilderTest, StructuresAndNpcsBlockTheirCells) {
    EXPECT_FALSE(town().is_walkable(Position{0, 0}));
    EXPECT_FALSE(town().is_walkable(Position{1, 0}));
    EXPECT_FALSE(town().is_walkable(Position{3, 0}));
    EXPECT_TRUE(town().is_walkable(Position{2, 0}));
}

TEST_F(WorldBuilderTest, PassagesStayWalkableButDoorsBlock) {
    EXPECT_TRUE(farm().is_walkable(Position{5, 4}));
    EXPECT_FALSE(town().is_walkable(Position{5, 0}));
    EXPECT_FALSE(level(0).is_walkable(Position{1, 0}));
}

TEST_F(WorldBuilderTest, TransitionIsAPortalOnEveryCellOfItsTrigger) {
    for (const Position& stairs: {Position{1, 0}, Position{2, 0}}) {
        std::optional<Portal> portal = level(0).portal_at(stairs);
        ASSERT_TRUE(portal.has_value());
        EXPECT_EQ(portal->target, &level(1));
        EXPECT_EQ(cell_of(portal->arrival), (Position{0, 0}));
    }
}

TEST_F(WorldBuilderTest, EveryLevelExitLeadsToTheTown) {
    std::optional<Portal> portal = level(Dungeon::LEVEL_COUNT - 1).portal_at(Position{0, 0});
    ASSERT_TRUE(portal.has_value());
    EXPECT_EQ(portal->target, &town());
    EXPECT_EQ(cell_of(portal->arrival), (Position{5, 1}));
    EXPECT_EQ(town().portal_at(Position{5, 0})->target, &level(0));
}

TEST_F(WorldBuilderTest, CabinBlocksAllItsCells) {
    EXPECT_FALSE(farm().is_walkable(Position{1, 1}));
    EXPECT_FALSE(farm().is_walkable(Position{3, 2}));
    EXPECT_FALSE(farm().is_walkable(Position{2, 2}));
    EXPECT_EQ(farm().get_cabin_pos(), (Position{2, 2}));
}

TEST_F(WorldBuilderTest, CabinDoorLeadsToTheInteriorAndBack) {
    std::optional<Portal> in = farm().portal_at(Position{2, 2});
    ASSERT_TRUE(in.has_value());
    Cabin& cabin = farm().get_cabin();
    EXPECT_EQ(in->target, &cabin);
    EXPECT_EQ(cell_of(in->arrival), cabin.get_door_pos());
    EXPECT_EQ(cabin.portal_at(cabin.get_door_pos())->target, &farm());
}

TEST_F(WorldBuilderTest, CabinHasABedSlotForEachPlayer) {
    EXPECT_EQ(farm().get_cabin().bed_slot_count(), server_config().game.max_players);
}

TEST(WorldBuilderErrorsTest, FarmWithoutCabinIsRejected) {
    maps::MapDefinition map = make_map();
    map.elements.erase(map.elements.begin());
    EXPECT_THROW(build_world(map), std::invalid_argument);
}

TEST(WorldBuilderErrorsTest, MissingDungeonLevelIsRejected) {
    maps::MapDefinition map = make_map();
    map.areas.erase(std::find_if(map.areas.begin(), map.areas.end(),
                                 [](const maps::MapArea& a) { return a.id == floor_id(10); }));
    EXPECT_THROW(build_world(map), std::invalid_argument);
}

TEST(WorldBuilderErrorsTest, TransitionToAnUnknownAreaIsRejected) {
    maps::MapDefinition map = make_map();
    map.transitions.front().toAreaId = "nowhere";
    EXPECT_THROW(build_world(map), std::invalid_argument);
}

TEST(WorldBuilderExampleTest, TheValidExampleMapBuilds) {
    maps::TomlMapRepository repository;
    maps::MapDefinition map = repository.loadMap(TALLER_MAPS_DIR "/examples/basic_valid.toml");
    EXPECT_NO_THROW(build_world(map));
}

}  // namespace
