#include <algorithm>
#include <optional>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "common/map/validation/MapValidator.h"
#include "editor/ElementHelper.h"
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

const maps::MapTransition* transitionOf(const maps::MapDefinition& map,
                                        const std::string& elementId) {
    auto transition =
            std::find_if(map.transitions.begin(), map.transitions.end(),
                         [&elementId](const auto& t) { return t.triggerElementId == elementId; });
    return transition == map.transitions.end() ? nullptr : &*transition;
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

TEST_F(MapEditingTest, RemovingAnElementTakesItOutOfTheMap) {
    editing.removeElement("mercado");

    EXPECT_EQ(elementById(editing.map(), "mercado"), nullptr);
    EXPECT_EQ(mapChanges, 2);
}

TEST_F(MapEditingTest, RemovingADoorRemovesTheWholePassage) {
    editing.removeElement("granja-porton");

    EXPECT_EQ(elementById(editing.map(), "granja-porton"), nullptr);
    EXPECT_EQ(elementById(editing.map(), "pueblo-porton"), nullptr);
    EXPECT_FALSE(triggers(editing.map(), "granja-porton"));
    EXPECT_FALSE(triggers(editing.map(), "pueblo-porton"));
}

TEST_F(MapEditingTest, RemovingAnEntranceTakesTheExitsToTheOtherOne) {
    // Otra entrada en (20, 17) del pueblo: se llega frente a ella, una fila mas abajo
    ASSERT_TRUE(editing.placeEntrance({VILLAGE_OFFSET_X + 20, 17}, std::nullopt));
    editing.removeElement("entrada-mina");

    EXPECT_EQ(elementById(editing.map(), "entrada-mina"), nullptr);
    const maps::MapTransition* exit = transitionOf(editing.map(), "mina-1-salida");
    ASSERT_NE(exit, nullptr);
    EXPECT_EQ(exit->toAreaId, "pueblo");
    EXPECT_EQ(exit->targetX, 21);
    EXPECT_GT(exit->targetY, 17);
}

TEST_F(MapEditingTest, MovingTheExitOfAFloorMovesItInEveryFloor) {
    // mina-1 empieza en la fila 20: (5, 3) del piso es (5, 23) del mapa general
    ASSERT_TRUE(editing.moveElement("mina-1-salida", {5, 23}));

    const maps::MapElement* lastExit = elementById(editing.map(), "mina-10-salida");
    ASSERT_NE(lastExit, nullptr);
    EXPECT_EQ(lastExit->x, 5);
    EXPECT_EQ(lastExit->y, 3);
    // La escalera del piso 1 deja al lado de la salida del piso 2
    const maps::MapTransition* stairs = transitionOf(editing.map(), "mina-1-escalera");
    ASSERT_NE(stairs, nullptr);
    EXPECT_EQ(stairs->targetX, 6);
    EXPECT_EQ(stairs->targetY, 3);
}

TEST_F(MapEditingTest, AnAreaDoesNotMoveOntoAnother) {
    EXPECT_FALSE(editing.moveArea("granja", 1, 0));
    EXPECT_EQ(mapChanges, 1);
}

TEST_F(MapEditingTest, MovingAnAreaAwayRemovesThePassagesThatGetSeparated) {
    ASSERT_TRUE(editing.moveArea("granja", 0, -25));

    EXPECT_EQ(areaById(editing.map(), "granja")->offsetY, -25);
    EXPECT_EQ(elementById(editing.map(), "granja-porton"), nullptr);
    EXPECT_EQ(elementById(editing.map(), "pueblo-porton"), nullptr);
}

TEST_F(MapEditingTest, TheFloorsOfTheMineMoveTogether) {
    ASSERT_TRUE(editing.moveArea("mina-3", 0, 40));

    EXPECT_EQ(areaById(editing.map(), "mina-1")->offsetY, 60);
    EXPECT_EQ(areaById(editing.map(), "mina-3")->offsetY, 60);
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

TEST_F(MapEditingTest, TheEntranceCreatesTheFloorsOfTheMineWhenThereAreNone) {
    editing.load(example_map_without_mine());
    // Donde estaba la entrada en valle.toml: (24, 2) del pueblo
    ASSERT_TRUE(editing.placeEntrance({VILLAGE_OFFSET_X + 24, 2}, AreaSize{20, 15}));

    const maps::MapDefinition& map = editing.map();
    EXPECT_EQ(std::count_if(map.areas.begin(), map.areas.end(),
                            [](const auto& a) { return a.type == maps::AreaType::DungeonFloor; }),
              10);
    EXPECT_NE(elementById(map, "mina-10-salida"), nullptr);
    EXPECT_EQ(elementById(map, "mina-10-escalera"), nullptr);
    // Con las transiciones de la entrada, las salidas y las escaleras el mapa queda valido
    for (const maps::MapValidationIssue& issue: maps::validateMap(map, example_requirements())) {
        ADD_FAILURE() << maps::toString(issue.code);
    }
}

TEST_F(MapEditingTest, AnotherEntranceOnlyLinksToTheFloorsThatAlreadyExist) {
    ASSERT_TRUE(editing.placeEntrance({VILLAGE_OFFSET_X + 20, 17}, AreaSize{20, 15}));

    EXPECT_EQ(editing.map().areas.size(), example_map().areas.size());
    EXPECT_TRUE(triggers(editing.map(), "dungeon_entrance-1"));
}

TEST_F(MapEditingTest, TheExitAndTheStairsOfAFloorAreNotRemoved) {
    editing.removeElement("mina-1-salida");
    editing.removeElement("mina-1-escalera");

    EXPECT_NE(elementById(editing.map(), "mina-1-salida"), nullptr);
    EXPECT_NE(elementById(editing.map(), "mina-1-escalera"), nullptr);
}

TEST_F(MapEditingTest, ANewAreaIsAddedEmptyWhereItDoesNotStepOnAnother) {
    EXPECT_FALSE(editing.placeArea(maps::AreaType::Farm, {10, 10}, {5, 5}));
    ASSERT_TRUE(editing.placeArea(maps::AreaType::Farm, {10, 8}, {100, 0}));

    // Ya hay una granja: la nueva es granja-2
    const maps::MapArea* farm = areaById(editing.map(), "granja-2");
    ASSERT_NE(farm, nullptr);
    EXPECT_EQ(farm->offsetX, 100);
    EXPECT_EQ(farm->width, 10);
    EXPECT_EQ(farm->height, 8);
    EXPECT_EQ(farm->tiles[7][9], maps::TileType::Grass);
}

TEST_F(MapEditingTest, RemovingAnAreaTakesItsElementsItsTransitionsAndItsPassages) {
    editing.removeArea("granja");

    const maps::MapDefinition& map = editing.map();
    EXPECT_EQ(areaById(map, "granja"), nullptr);
    EXPECT_EQ(elementById(map, "cabana"), nullptr);
    // El porton del pueblo era del pasaje con la granja
    EXPECT_EQ(elementById(map, "pueblo-porton"), nullptr);
    EXPECT_TRUE(std::none_of(map.transitions.begin(), map.transitions.end(), [](const auto& t) {
        return t.fromAreaId == "granja" || t.toAreaId == "granja";
    }));
}

TEST_F(MapEditingTest, RemovingAFloorRemovesTheWholeMineButNotTheEntrance) {
    editing.removeArea("mina-3");

    const maps::MapDefinition& map = editing.map();
    EXPECT_TRUE(std::none_of(map.areas.begin(), map.areas.end(),
                             [](const auto& a) { return a.type == maps::AreaType::DungeonFloor; }));
    EXPECT_NE(elementById(map, "entrada-mina"), nullptr);
    EXPECT_FALSE(triggers(map, "entrada-mina"));
}

TEST_F(MapEditingTest, TheEntranceOfANewVillageTakesTheExitsOfTheMine) {
    editing.removeArea("pueblo");
    ASSERT_TRUE(editing.placeArea(maps::AreaType::Village, {30, 20}, {VILLAGE_OFFSET_X, 0}));
    ASSERT_TRUE(editing.placeEntrance({VILLAGE_OFFSET_X + 24, 2}, std::nullopt));

    const maps::MapTransition* exit = transitionOf(editing.map(), "mina-1-salida");
    ASSERT_NE(exit, nullptr);
    EXPECT_EQ(exit->toAreaId, "pueblo");
    EXPECT_GT(exit->targetY, 2);
    EXPECT_TRUE(triggers(editing.map(), "dungeon_entrance-1"));
}
