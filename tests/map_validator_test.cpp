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

TEST(MapValidatorDoorTest, ATreeInFrontOfTheDoorIsRejected) {
    maps::MapDefinition map = example_map();
    const maps::MapElement market = element_by_id(map, "mercado");
    map.elements.push_back({"arbol-en-la-puerta", market.areaId, maps::ElementType::Tree,
                            maps::ElementLayer::Structure, market.x + market.width / 2,
                            market.y + market.height, 2, 1});

    EXPECT_TRUE(has_blocked_door(maps::validateMap(map, requirements()), "mercado"));
}

TEST(MapValidatorDoorTest, WaterInFrontOfTheSecondHalfOfADoubleDoorIsRejected) {
    maps::MapDefinition map = example_map();
    const maps::MapElement market = element_by_id(map, "mercado");
    const maps::MapCell right = maps::doorCells(market).back();
    area_by_id(map, "pueblo").tiles[right.y + 1][right.x] = maps::TileType::Water;

    EXPECT_TRUE(has_blocked_door(maps::validateMap(map, requirements()), "mercado"));
}

TEST(MapValidatorDoorTest, ADoorOnTheBottomEdgeOfTheAreaIsRejected) {
    maps::MapDefinition map = example_map();
    maps::MapElement& center = element_by_id(map, "centro-comunitario");
    center.y = area_by_id(map, "pueblo").height - center.height;

    EXPECT_TRUE(has_blocked_door(maps::validateMap(map, requirements()), "centro-comunitario"));
}

// La puerta como esta en cada dibujo: la cabaña simple, el mercado y el centro comunitario dobles
TEST(DoorCellsTest, EachBuildingHasItsDoorWhereItsSpriteDrawsIt) {
    auto columns = [](maps::ElementType type, int width) {
        std::vector<int> xs;
        for (const maps::MapCell& cell:
             maps::doorCells({"b", "a", type, maps::ElementLayer::Structure, 10, 4, width, 3})) {
            EXPECT_EQ(cell.y, 6);
            xs.push_back(cell.x - 10);
        }
        return xs;
    };
    EXPECT_EQ(columns(maps::ElementType::Cabin, 5), (std::vector<int>{2}));
    EXPECT_EQ(columns(maps::ElementType::Market, 10), (std::vector<int>{5, 6}));
    EXPECT_EQ(columns(maps::ElementType::CommunityCenter, 12), (std::vector<int>{5, 6}));
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

namespace {

bool has_issue(const std::vector<maps::MapValidationIssue>& issues, maps::MapIssueCode code) {
    return std::any_of(issues.begin(), issues.end(),
                       [code](const auto& issue) { return issue.code == code; });
}

}  // namespace

TEST(MapValidatorRulesTest, ACabinInTheVillageIsRejected) {
    maps::MapDefinition map = example_map();
    maps::MapElement& cabin = element_by_id(map, "cabana");
    cabin.areaId = "pueblo";
    cabin.x = 1;
    cabin.y = 15;

    EXPECT_TRUE(has_issue(maps::validateMap(map, requirements()),
                          maps::MapIssueCode::ElementInWrongArea));
}

TEST(MapValidatorRulesTest, ABuildingWithAnotherSizeIsRejected) {
    maps::MapDefinition map = example_map();
    element_by_id(map, "mercado").width = 4;

    EXPECT_TRUE(has_issue(maps::validateMap(map, requirements()),
                          maps::MapIssueCode::WrongElementSize));
}

TEST(MapValidatorRulesTest, ASecondMarketIsRejected) {
    maps::MapDefinition map = example_map();
    maps::MapElement copy = element_by_id(map, "mercado");
    copy.id = "otro-mercado";
    copy.y += copy.height + 1;
    map.elements.push_back(copy);

    EXPECT_TRUE(
            has_issue(maps::validateMap(map, requirements()), maps::MapIssueCode::TooManyElements));
}

TEST(MapValidatorRulesTest, ACaveFloorInTheFarmIsRejected) {
    maps::MapDefinition map = example_map();
    area_by_id(map, "granja").tiles[0][0] = maps::TileType::CaveFloor;

    EXPECT_TRUE(
            has_issue(maps::validateMap(map, requirements()), maps::MapIssueCode::TileInWrongArea));
}

TEST(MapValidatorRulesTest, RocksHaveNoLimit) {
    maps::MapDefinition map = example_map();
    maps::MapElement copy = element_by_id(map, "mina-1-roca-1");
    copy.id = "mina-1-roca-3";
    copy.x += 2;
    map.elements.push_back(copy);

    EXPECT_TRUE(maps::validateMap(map, requirements()).empty());
}
