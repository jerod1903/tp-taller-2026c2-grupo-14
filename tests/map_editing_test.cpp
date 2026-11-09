#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "editor/MapEditing.h"
#include "editor/MapHelper.h"
#include "tests/test_worlds.h"

namespace {

namespace maps = stardew::maps;

// En valle.toml el pueblo empieza en la columna 30 del mapa general
constexpr int VILLAGE_OFFSET_X = 30;

bool triggers(const maps::MapDefinition& map, const std::string& elementId) {
    return std::any_of(map.transitions.begin(), map.transitions.end(),
                       [&elementId](const auto& t) { return t.triggerElementId == elementId; });
}

class MapEditingTest: public ::testing::Test {
protected:
    void SetUp() override {
        editing.onTileChanged([this](const maps::MapCell& cell) { changedTiles.push_back(cell); });
        editing.onMapChanged([this]() { ++mapChanges; });
        editing.load(example_map());
    }

    MapEditing editing{example_requirements()};
    std::vector<maps::MapCell> changedTiles;
    int mapChanges = 0;
};

}  // namespace

TEST_F(MapEditingTest, LoadingAMapNotifiesThatEverythingChanged) { EXPECT_EQ(mapChanges, 1); }

TEST_F(MapEditingTest, ACellOfTheGeneralMapIsTranslatedToItsArea) {
    std::optional<AreaCell> cell = cellAt(editing.map(), VILLAGE_OFFSET_X + 6, 12);

    ASSERT_TRUE(cell.has_value());
    EXPECT_EQ(editing.map().areas[cell->area].id, "pueblo");
    EXPECT_EQ(cell->x, 6);
    EXPECT_EQ(cell->y, 12);
}

TEST_F(MapEditingTest, OutsideEveryAreaThereIsNoCell) {
    EXPECT_FALSE(cellAt(editing.map(), -1, 0).has_value());
}

TEST_F(MapEditingTest, PaintingChangesTheTileOfThatAreaAndNotifiesTheCell) {
    editing.paint({VILLAGE_OFFSET_X + 6, 12}, maps::TileType::Water);

    EXPECT_EQ(editing.map().areas[1].tiles[12][6], maps::TileType::Water);
    ASSERT_EQ(changedTiles.size(), 1u);
    EXPECT_EQ(changedTiles[0].x, VILLAGE_OFFSET_X + 6);
    EXPECT_EQ(changedTiles[0].y, 12);
}

TEST_F(MapEditingTest, PaintingTheSameTileAgainDoesNotNotify) {
    editing.paint({VILLAGE_OFFSET_X + 6, 12}, maps::TileType::Grass);

    EXPECT_TRUE(changedTiles.empty());
}

TEST_F(MapEditingTest, PaintingOutsideEveryAreaDoesNothing) {
    editing.paint({-1, 0}, maps::TileType::Water);

    EXPECT_TRUE(changedTiles.empty());
    EXPECT_EQ(editing.map(), example_map());
}

TEST_F(MapEditingTest, APassageNotifiesThatTheMapChanged) {
    editing.togglePassage({29, 12});

    EXPECT_EQ(mapChanges, 2);
}

TEST_F(MapEditingTest, AClickThatDoesNotMakeAPassageDoesNotNotify) {
    editing.togglePassage({10, 10});

    EXPECT_EQ(mapChanges, 1);
}

TEST_F(MapEditingTest, RemovingAnElementAlsoRemovesItsTransitions) {
    editing.removeElement("mina-1-escalera");

    EXPECT_EQ(elementById(editing.map(), "mina-1-escalera"), nullptr);
    EXPECT_FALSE(triggers(editing.map(), "mina-1-escalera"));
    EXPECT_EQ(mapChanges, 2);
}

TEST_F(MapEditingTest, RemovingADoorRemovesTheWholePassage) {
    editing.removeElement("granja-porton");

    EXPECT_EQ(elementById(editing.map(), "granja-porton"), nullptr);
    EXPECT_EQ(elementById(editing.map(), "pueblo-porton"), nullptr);
    EXPECT_FALSE(triggers(editing.map(), "granja-porton"));
    EXPECT_FALSE(triggers(editing.map(), "pueblo-porton"));
}

TEST_F(MapEditingTest, TheDungeonEntranceIsNotRemoved) {
    editing.removeElement("entrada-mina");

    EXPECT_NE(elementById(editing.map(), "entrada-mina"), nullptr);
    EXPECT_EQ(mapChanges, 1);
}

TEST_F(MapEditingTest, AnElementIsPlacedWithTheSizeOfItsRuleAndItsBaseCornerOnTheCell) {
    // La cabana mide 5 x 3: con la esquina de abajo a la izquierda en (2, 18) la base arranca en
    // (2, 16)
    ASSERT_TRUE(editing.placeElement(maps::ElementType::Cabin, {2, 18}));

    const maps::MapElement* cabin = elementById(editing.map(), "cabin-1");
    ASSERT_NE(cabin, nullptr);
    EXPECT_EQ(cabin->areaId, "granja");
    EXPECT_EQ(cabin->x, 2);
    EXPECT_EQ(cabin->y, 16);
    EXPECT_EQ(cabin->width, 5);
    EXPECT_EQ(cabin->height, 3);
    EXPECT_EQ(mapChanges, 2);
}

TEST_F(MapEditingTest, TheIdTakesTheFirstFreeNumber) {
    ASSERT_TRUE(editing.placeElement(maps::ElementType::OreNode, {5, 25}));
    ASSERT_TRUE(editing.placeElement(maps::ElementType::OreNode, {6, 25}));

    EXPECT_NE(elementById(editing.map(), "ore_node-1"), nullptr);
    EXPECT_NE(elementById(editing.map(), "ore_node-2"), nullptr);
}

TEST_F(MapEditingTest, AnElementIsNotPlacedOverAnotherOne) {
    // mina-1 empieza en la fila 20: su mineral de (4, 11) esta en (4, 31) del mapa general
    EXPECT_FALSE(editing.placeElement(maps::ElementType::OreNode, {4, 31}));
    EXPECT_EQ(mapChanges, 1);
}

TEST_F(MapEditingTest, AnElementIsNotPlacedIfItsBaseLeavesTheArea) {
    // El mercado mide 10 de ancho: desde la columna 25 del pueblo se sale por la derecha
    EXPECT_FALSE(editing.placeElement(maps::ElementType::Market, {VILLAGE_OFFSET_X + 25, 15}));
    EXPECT_EQ(mapChanges, 1);
}
