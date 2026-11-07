#include <algorithm>
#include <cstdio>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "common/map/persistence/MapRequirementsLoader.h"
#include "common/map/persistence/TomlMapFile.h"
#include "common/map/validation/MapValidator.h"
#include "tests/test_worlds.h"

namespace {

namespace maps = stardew::maps;

const maps::MapRequirements& requirements() {
    static const maps::MapRequirements loaded =
            maps::loadMapRequirements(TALLER_MAPS_DIR "/config/map_requirements.toml");
    return loaded;
}

maps::MapElement& element_by_id(maps::MapDefinition& map, const std::string& id) {
    return *std::find_if(map.elements.begin(), map.elements.end(),
                         [&id](const maps::MapElement& e) { return e.id == id; });
}

maps::MapArea& area_by_id(maps::MapDefinition& map, const std::string& id) {
    return *std::find_if(map.areas.begin(), map.areas.end(),
                         [&id](const maps::MapArea& a) { return a.id == id; });
}

bool has_blocked_door(const std::vector<maps::MapValidationIssue>& issues,
                      const std::string& building) {
    return std::any_of(issues.begin(), issues.end(), [&building](const auto& issue) {
        return issue.code == maps::MapIssueCode::BlockedDoor && issue.elementId == building;
    });
}

}  // namespace

TEST(MapValidatorTest, TheExampleMapIsValid) {
    EXPECT_TRUE(maps::validateMap(example_map(), requirements()).empty());
}

TEST(MapValidatorDoorTest, WaterInFrontOfTheDoorIsRejected) {
    maps::MapDefinition map = example_map();
    const maps::MapElement cabin = element_by_id(map, "cabana");
    area_by_id(map, "granja").tiles[cabin.y + cabin.height][cabin.x + cabin.width / 2] =
            maps::TileType::Water;

    EXPECT_TRUE(has_blocked_door(maps::validateMap(map, requirements()), "cabana"));
}

TEST(MapValidatorDoorTest, AnNpcInFrontOfTheDoorIsRejected) {
    maps::MapDefinition map = example_map();
    const maps::MapElement market = element_by_id(map, "mercado");
    maps::MapElement& marnie = element_by_id(map, "marnie");
    marnie.x = market.x + market.width / 2;
    marnie.y = market.y + market.height;

    EXPECT_TRUE(has_blocked_door(maps::validateMap(map, requirements()), "mercado"));
}

TEST(MapValidatorDoorTest, ADoorOnTheBottomEdgeOfTheAreaIsRejected) {
    maps::MapDefinition map = example_map();
    maps::MapElement& center = element_by_id(map, "centro-comunitario");
    center.y = area_by_id(map, "pueblo").height - center.height;

    EXPECT_TRUE(has_blocked_door(maps::validateMap(map, requirements()), "centro-comunitario"));
}

TEST(TomlMapFileTest, AreaOffsetsSurviveSavingAndLoading) {
    maps::MapDefinition map = example_map();
    area_by_id(map, "pueblo").offsetX = 42;
    area_by_id(map, "pueblo").offsetY = 7;
    const std::string path = testing::TempDir() + "offsets.toml";

    maps::saveTomlMapDefinition(path, map);
    maps::MapDefinition loaded = maps::loadTomlMapDefinition(path);
    std::remove(path.c_str());

    EXPECT_EQ(loaded, map);
}

TEST(MapValidatorSpriteTest, ARoofCutByTheTopEdgeIsRejected) {
    maps::MapDefinition map = example_map();
    element_by_id(map, "cabana").y = 0;

    std::vector<maps::MapValidationIssue> issues = maps::validateMap(map, requirements());
    EXPECT_TRUE(std::any_of(issues.begin(), issues.end(), [](const auto& issue) {
        return issue.code == maps::MapIssueCode::SpriteOutsideArea && issue.elementId == "cabana";
    }));
}
