#include "server/game/world/grid.h"

#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "server/game/world/direction.h"
#include "tests/world_helpers.h"

namespace {

TEST(DirectionTest, StepMovesOneCellInEachDirection) {
    const Position origin{2, 2};
    EXPECT_EQ(step(origin, Direction::UP), (Position{2, 1}));
    EXPECT_EQ(step(origin, Direction::DOWN), (Position{2, 3}));
    EXPECT_EQ(step(origin, Direction::LEFT), (Position{1, 2}));
    EXPECT_EQ(step(origin, Direction::RIGHT), (Position{3, 2}));
}

TEST(DirectionTest, StepWithNoneKeepsThePosition) {
    EXPECT_EQ(step(Position{2, 2}, Direction::NONE), (Position{2, 2}));
}

TEST(GridTest, ReportsItsSize) {
    Grid grid = make_grid(3, 4);
    EXPECT_EQ(grid.get_width(), 3);
    EXPECT_EQ(grid.get_height(), 4);
}

TEST(GridTest, CornersAreInBounds) {
    Grid grid = make_grid(3, 4);
    EXPECT_TRUE(grid.in_bounds(Position{0, 0}));
    EXPECT_TRUE(grid.in_bounds(Position{2, 3}));
}

TEST(GridTest, OutsideIsOutOfBounds) {
    Grid grid = make_grid(3, 4);
    EXPECT_FALSE(grid.in_bounds(Position{-1, 0}));
    EXPECT_FALSE(grid.in_bounds(Position{0, -1}));
    EXPECT_FALSE(grid.in_bounds(Position{3, 0}));
    EXPECT_FALSE(grid.in_bounds(Position{0, 4}));
}

TEST(GridTest, TilesAreStoredByRows) {
    // En una grilla de 3x2, (2, 1) es el último tile y (1, 2) estaría afuera.
    Grid grid = make_grid(3, 2, {Position{2, 1}});
    EXPECT_FALSE(grid.at(Position{2, 1}).is_walkable());
    EXPECT_EQ(grid.at(Position{2, 1}).get_type(), TileType::CaveWall);
    EXPECT_TRUE(grid.at(Position{1, 1}).is_walkable());
}

TEST(GridTest, RejectsTileCountThatDoesNotMatchTheSize) {
    std::vector<Tile> tiles(5, Tile(TileType::Dirt, true));
    EXPECT_THROW(Grid(2, 3, tiles), std::invalid_argument);
}

TEST(GridTest, RejectsNegativeSize) {
    std::vector<Tile> tiles(1, Tile(TileType::Dirt, true));
    EXPECT_THROW(Grid(-1, -1, tiles), std::invalid_argument);
}

TEST(TileTest, WaterWallsAndStoneAreNotWalkable) {
    EXPECT_FALSE(is_walkable_terrain(TileType::Water));
    EXPECT_FALSE(is_walkable_terrain(TileType::CaveWall));
    EXPECT_FALSE(is_walkable_terrain(TileType::Stone));
    EXPECT_FALSE(is_walkable_terrain(TileType::Wall));
    EXPECT_FALSE(is_walkable_terrain(TileType::Void));
}

TEST(TileTest, FloorsAreWalkable) {
    for (TileType type:
         {TileType::Grass, TileType::Dirt, TileType::TilledSoil, TileType::Path, TileType::Sand,
          TileType::WoodFloor, TileType::CaveFloor, TileType::HouseFloor, TileType::Sleepable}) {
        EXPECT_TRUE(is_walkable_terrain(type)) << stardew::maps::toString(type);
    }
}

}  // namespace
