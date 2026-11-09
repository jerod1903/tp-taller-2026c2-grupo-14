#include <optional>
#include <vector>

#include <gtest/gtest.h>

#include "editor/ElementHelper.h"
#include "editor/MapEditing.h"
#include "editor/tools/PlaceTool.h"
#include "tests/test_worlds.h"

namespace {

namespace maps = stardew::maps;

// En valle.toml la mina-1 empieza en la fila 20 y tiene un mineral en (4, 11)
constexpr maps::MapCell MINERAL{4, 31};
constexpr maps::MapCell FREE_IN_DUNGEON{5, 25};
constexpr maps::MapCell OUTSIDE_EVERY_AREA{200, 200};

class PlaceToolTest: public ::testing::Test {
protected:
    void SetUp() override {
        editing.load(example_map());
        tool.setType(maps::ElementType::OreNode);
        tool.onGhostChanged([this](const std::optional<Ghost>& ghost) { ghosts.push_back(ghost); });
        tool.onDone([this]() { ++doneCount; });
    }

    MapEditing editing{example_requirements()};
    PlaceTool tool{editing};
    std::vector<std::optional<Ghost>> ghosts;
    int doneCount = 0;
};

}  // namespace

TEST_F(PlaceToolTest, ClickingWhereItFitsPlacesItAndFinishes) {
    tool.onPress(FREE_IN_DUNGEON);

    EXPECT_NE(elementById(editing.map(), "ore_node-1"), nullptr);
    EXPECT_EQ(doneCount, 1);
    ASSERT_FALSE(ghosts.empty());
    EXPECT_FALSE(ghosts.back().has_value());
}

TEST_F(PlaceToolTest, ClickingWhereItDoesNotFitDoesNothing) {
    tool.onPress(MINERAL);

    EXPECT_EQ(elementById(editing.map(), "ore_node-1"), nullptr);
    EXPECT_EQ(doneCount, 0);
}

TEST_F(PlaceToolTest, TheGhostSaysWhetherItFitsAndHidesOutsideEveryArea) {
    tool.onHover(FREE_IN_DUNGEON);
    tool.onHover(MINERAL);
    tool.onHover(OUTSIDE_EVERY_AREA);

    ASSERT_EQ(ghosts.size(), 3U);
    ASSERT_TRUE(ghosts[0].has_value());
    EXPECT_TRUE(ghosts[0]->fits);
    ASSERT_TRUE(ghosts[1].has_value());
    EXPECT_FALSE(ghosts[1]->fits);
    EXPECT_FALSE(ghosts[2].has_value());
}
