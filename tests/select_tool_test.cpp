#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/ElementHelper.h"
#include "editor/MapEditing.h"
#include "editor/tools/SelectTool.h"
#include "tests/test_worlds.h"

namespace {

namespace maps = stardew::maps;

// En valle.toml el pueblo empieza en la columna 30: el mercado esta en (1, 6) del pueblo y mide
// 10 x 3
constexpr maps::MapCell MARKET{31, 6};
constexpr maps::MapCell MARKET_BOTTOM_LEFT{31, 8};
// mina-1 empieza en la fila 20 y tiene su salida en (1, 1)
constexpr maps::MapCell FIRST_FLOOR_EXIT{1, 21};

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

TEST_F(SelectToolTest, TheExitOfAFloorStaysSelectedBecauseItIsNotRemoved) {
    tool.onPress(FIRST_FLOOR_EXIT);
    tool.removeSelected();

    EXPECT_NE(elementById(editing.map(), "mina-1-salida"), nullptr);
    ASSERT_EQ(notified.size(), 1U);
    EXPECT_EQ(notified[0], "mina-1-salida");
}

TEST_F(SelectToolTest, DraggingAnElementMovesItWhereItIsDropped) {
    // El mercado mide 3 de alto: agarrado de su esquina de abajo y soltado en la fila 15, su base
    // arranca en la 13
    tool.onPress(MARKET_BOTTOM_LEFT);
    tool.onMove({MARKET_BOTTOM_LEFT.x, 15});
    tool.onRelease({MARKET_BOTTOM_LEFT.x, 15});

    const maps::MapElement* market = elementById(editing.map(), "mercado");
    ASSERT_NE(market, nullptr);
    EXPECT_EQ(market->x, 1);
    EXPECT_EQ(market->y, 13);
}
