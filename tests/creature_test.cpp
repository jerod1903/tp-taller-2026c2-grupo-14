#include "server/game/characters/creature.h"

#include <cmath>

#include <gtest/gtest.h>

#include "common/game_dto.h"
#include "tests/world_helpers.h"

namespace {

class CreatureTest: public ::testing::Test {
protected:
    // Pared en (2, 0).
    TestLocation location{make_grid(5, 5, {Position{2, 0}})};
    TestCreature creature{location};
};

TEST_F(CreatureTest, StartsOnTheCenterOfItsCell) {
    EXPECT_FLOAT_EQ(creature.get_x(), 0.5f);
    EXPECT_FLOAT_EQ(creature.get_y(), 0.5f);
}

TEST_F(CreatureTest, WithoutDirectionItStaysStill) {
    creature.update(1.0f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
    EXPECT_FLOAT_EQ(creature.get_y(), 0.5f);
    EXPECT_FALSE(creature.has_stepped());
}

TEST_F(CreatureTest, MovesSpeedTimesDtCells) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.25f);
    EXPECT_FLOAT_EQ(creature.get_y(), 0.75f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
}

TEST_F(CreatureTest, ItsCellIsTheOneUnderItsCenter) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.75f);
    EXPECT_EQ(creature.get_position(), (Position{0, 1}));
}

TEST_F(CreatureTest, SpeedScalesTheMovement) {
    TestCreature fast(location, Position{0, 0}, 2.0f);
    fast.set_direction(Direction::DOWN);
    fast.update(0.5f);
    EXPECT_FLOAT_EQ(fast.get_y(), 1.5f);
}

TEST_F(CreatureTest, StopsAtOnce) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.25f);
    creature.set_direction(Direction::NONE);
    creature.update(1.0f);
    EXPECT_FLOAT_EQ(creature.get_y(), 0.75f);
}

TEST_F(CreatureTest, TurnsAtOnce) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.25f);
    creature.set_direction(Direction::RIGHT);
    creature.update(0.25f);
    EXPECT_FLOAT_EQ(creature.get_x(), 0.75f);
    EXPECT_FLOAT_EQ(creature.get_y(), 0.75f);
}

TEST_F(CreatureTest, DiagonalsGoAtTheSameSpeed) {
    TestCreature creature_in_the_middle(location, Position{2, 2});
    creature_in_the_middle.set_direction(Direction::DOWN_RIGHT);
    creature_in_the_middle.update(1.0f);
    float dx = creature_in_the_middle.get_x() - 2.5f;
    float dy = creature_in_the_middle.get_y() - 2.5f;
    EXPECT_FLOAT_EQ(dx, dy);
    EXPECT_NEAR(std::sqrt(dx * dx + dy * dy), 1.0f, 1e-5f);
}

TEST_F(CreatureTest, BordersStopItAgainstThem) {
    creature.set_direction(Direction::UP);
    creature.update(1.0f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
    EXPECT_FLOAT_EQ(creature.get_y(), CHARACTER_HALF_HEIGHT);
}

TEST_F(CreatureTest, WallsStopItAgainstThem) {
    creature.set_direction(Direction::RIGHT);
    creature.update(2.0f);
    EXPECT_EQ(creature.get_position(), (Position{1, 0}));
    EXPECT_FLOAT_EQ(creature.get_x(), 2.0f - CHARACTER_HALF_WIDTH);
}

TEST_F(CreatureTest, FastCreaturesDoNotGoThroughWalls) {
    TestCreature fast(location, Position{0, 0}, 10.0f);
    fast.set_direction(Direction::RIGHT);
    fast.update(1.0f);
    EXPECT_EQ(fast.get_position(), (Position{1, 0}));
}

TEST_F(CreatureTest, SlidesAlongWhatStopsIt) {
    creature.set_direction(Direction::UP_RIGHT);
    creature.update(0.5f);
    EXPECT_FLOAT_EQ(creature.get_y(), CHARACTER_HALF_HEIGHT);
    EXPECT_GT(creature.get_x(), 0.5f);
}

TEST_F(CreatureTest, AgainstAWallItCanStillTurn) {
    creature.set_direction(Direction::RIGHT);
    creature.update(2.0f);
    creature.set_direction(Direction::DOWN);
    creature.update(1.0f);
    EXPECT_EQ(creature.get_position(), (Position{1, 1}));
}

TEST_F(CreatureTest, HasSteppedOnlyOnTheTickItEntersACell) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.75f);
    EXPECT_TRUE(creature.has_stepped());
    creature.update(0.25f);
    EXPECT_FALSE(creature.has_stepped());
}

TEST_F(CreatureTest, DamageDoesNotGoBelowZero) {
    creature.take_damage(4);
    EXPECT_EQ(creature.get_health(), 6);
    EXPECT_TRUE(creature.is_alive());
    creature.take_damage(100);
    EXPECT_EQ(creature.get_health(), 0);
    EXPECT_FALSE(creature.is_alive());
}

TEST_F(CreatureTest, RelocateMovesItToTheOtherLocation) {
    TestLocation other;
    location.add_entity(creature);
    creature.relocate(other, Position{3, 3});
    EXPECT_EQ(&creature.get_location(), &other);
    EXPECT_EQ(creature.get_position(), (Position{3, 3}));
    EXPECT_FLOAT_EQ(creature.get_x(), 3.5f);
    EXPECT_FLOAT_EQ(creature.get_y(), 3.5f);
    EXPECT_TRUE(location.get_entities().empty());
    EXPECT_EQ(other.get_entities().size(), 1u);
}

}  // namespace
