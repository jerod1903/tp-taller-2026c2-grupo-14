#include "editor/Passages.h"

#include <algorithm>
#include <string>

#include <gtest/gtest.h>

#include "common/map/persistence/MapRequirementsLoader.h"
#include "common/map/validation/MapValidator.h"
#include "tests/test_worlds.h"

namespace {

namespace maps = stardew::maps;

// En valle.toml la granja va de x=0 a x=29 y el pueblo de x=30 a x=59: el borde entre las dos
// esta entre las columnas 29 y 30 del mapa general.
constexpr int FARM_EDGE = 29;
constexpr int VILLAGE_EDGE = 30;

const maps::MapElement* element(const maps::MapDefinition& map, const std::string& id) {
    auto found = std::find_if(map.elements.begin(), map.elements.end(),
                              [&id](const auto& e) { return e.id == id; });
    return found == map.elements.end() ? nullptr : &*found;
}

const maps::MapTransition* transition(const maps::MapDefinition& map, const std::string& id) {
    auto found = std::find_if(map.transitions.begin(), map.transitions.end(),
                              [&id](const auto& t) { return t.id == id; });
    return found == map.transitions.end() ? nullptr : &*found;
}

bool valid(const maps::MapDefinition& map) {
    static const maps::MapRequirements requirements =
            maps::loadMapRequirements(TALLER_MAPS_DIR "/config/map_requirements.toml");
    return maps::validateMap(map, requirements).empty();
}

}  // namespace

TEST(PassagesTest, ClickingTheBorderCreatesADoorOnEachSideAndBothTransitions) {
    maps::MapDefinition map = example_map();

    ASSERT_TRUE(togglePassage(map, FARM_EDGE, 12));

    const maps::MapElement* farmDoor = element(map, "granja-pasaje-1");
    const maps::MapElement* villageDoor = element(map, "pueblo-pasaje-1");
    ASSERT_NE(farmDoor, nullptr);
    ASSERT_NE(villageDoor, nullptr);
    EXPECT_EQ(farmDoor->x, 29);
    EXPECT_EQ(villageDoor->x, 0);
    // Cada porton deja al jugador una casilla adentro del otro lado, no encima del otro porton
    const maps::MapTransition* toVillage = transition(map, "granja-pasaje-1-ruta");
    const maps::MapTransition* toFarm = transition(map, "pueblo-pasaje-1-ruta");
    ASSERT_NE(toVillage, nullptr);
    ASSERT_NE(toFarm, nullptr);
    EXPECT_EQ(toVillage->toAreaId, "pueblo");
    EXPECT_EQ(toVillage->targetX, 1);
    EXPECT_EQ(toVillage->targetY, 12);
    EXPECT_EQ(toFarm->toAreaId, "granja");
    EXPECT_EQ(toFarm->targetX, 28);
    EXPECT_TRUE(valid(map));
}

TEST(PassagesTest, ClickingEitherSideOfAPassageRemovesItWhole) {
    maps::MapDefinition map = example_map();

    ASSERT_TRUE(togglePassage(map, VILLAGE_EDGE, 10));

    EXPECT_EQ(element(map, "granja-porton"), nullptr);
    EXPECT_EQ(element(map, "pueblo-porton"), nullptr);
    EXPECT_EQ(transition(map, "granja-a-pueblo"), nullptr);
    EXPECT_EQ(transition(map, "pueblo-a-granja"), nullptr);
}

TEST(PassagesTest, AwayFromTheBorderNothingChanges) {
    maps::MapDefinition map = example_map();

    EXPECT_FALSE(togglePassage(map, 10, 10));
    EXPECT_EQ(map, example_map());
}

TEST(PassagesTest, APassageIntoWaterIsNotCreated) {
    maps::MapDefinition map = example_map();
    map.areas[1].tiles[12][0] = maps::TileType::Water;
    const maps::MapDefinition before = map;

    EXPECT_FALSE(togglePassage(map, FARM_EDGE, 12));
    EXPECT_EQ(map, before);
}
