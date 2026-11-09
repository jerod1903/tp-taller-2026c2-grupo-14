#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/MapEditing.h"
#include "editor/MapHelper.h"
#include "editor/tools/SelectTool.h"
#include "tests/test_worlds.h"

namespace {

namespace maps = stardew::maps;

// En valle.toml el pueblo empieza en la columna 30: el mercado esta en (1, 6) del pueblo y la
// entrada a la mina en (24, 2)
constexpr maps::MapCell MARKET{31, 6};
constexpr maps::MapCell DUNGEON_ENTRANCE{54, 2};

class SelectToolTest: public ::testing::Test {
protected:
    void SetUp() override {
        editing.load(example_map());
        tool.onSelectionChanged(
                [this](const std::optional<std::string>& id) { notified.push_back(id); });
    }

    MapEditing editing{example_requirements()};
    SelectTool tool{editing};
    std::vector<std::optional<std::string>> notified;
};

}  // namespace

TEST_F(SelectToolTest, ClickingAnElementSelectsItAndAFreeCellClearsIt) {
    tool.onPress(MARKET);
    tool.onPress({10, 10});

    ASSERT_EQ(notified.size(), 2U);
    EXPECT_EQ(notified[0], "mercado");
    EXPECT_EQ(notified[1], std::nullopt);
}

TEST_F(SelectToolTest, RemovingTheSelectedElementClearsTheSelection) {
    tool.onPress(MARKET);
    tool.removeSelected();

    EXPECT_EQ(elementById(editing.map(), "mercado"), nullptr);
    EXPECT_EQ(notified.back(), std::nullopt);
}

TEST_F(SelectToolTest, TheDungeonEntranceStaysSelectedBecauseItIsNotRemoved) {
    tool.onPress(DUNGEON_ENTRANCE);
    tool.removeSelected();

    EXPECT_NE(elementById(editing.map(), "entrada-mina"), nullptr);
    ASSERT_EQ(notified.size(), 1U);
    EXPECT_EQ(notified[0], "entrada-mina");
}
