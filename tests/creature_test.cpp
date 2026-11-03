#include "server/game/characters/creature.h"

#include <gtest/gtest.h>

#include "tests/world_helpers.h"

namespace {

class CreatureTest: public ::testing::Test {
protected:
    // Pared en (2, 0).
    TestLocation location{make_grid(5, 5, {Position{2, 0}})};
    TestCreature creature{location};
};

TEST_F(CreatureTest, WithoutDirectionItStaysStill) {
    creature.update(1.0f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
    EXPECT_FALSE(creature.has_stepped());
}

TEST_F(CreatureTest, MovesOnceItAccumulatesACell) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.5f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
    creature.update(0.5f);
    EXPECT_EQ(creature.get_position(), (Position{0, 1}));
}

TEST_F(CreatureTest, SpeedScalesTheProgress) {
    TestCreature fast(location, Position{0, 0}, 2.0f);
    fast.set_direction(Direction::DOWN);
    fast.update(0.5f);
    EXPECT_EQ(fast.get_position(), (Position{0, 1}));
}

TEST_F(CreatureTest, KeepsTheLeftoverProgress) {
    creature.set_direction(Direction::DOWN);
    creature.update(1.5f);
    creature.update(0.5f);
    EXPECT_EQ(creature.get_position(), (Position{0, 2}));
}

TEST_F(CreatureTest, BordersStopIt) {
    creature.set_direction(Direction::UP);
    creature.update(1.0f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
}

TEST_F(CreatureTest, WallsStopIt) {
    creature.set_direction(Direction::RIGHT);
    creature.update(1.0f);
    creature.update(1.0f);
    EXPECT_EQ(creature.get_position(), (Position{1, 0}));
    EXPECT_FALSE(creature.has_stepped());
}

TEST_F(CreatureTest, ChangingDirectionResetsTheProgress) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.5f);
    creature.set_direction(Direction::RIGHT);
    creature.update(0.5f);
    EXPECT_EQ(creature.get_position(), (Position{0, 0}));
}

TEST_F(CreatureTest, KeepingTheDirectionKeepsTheProgress) {
    creature.set_direction(Direction::DOWN);
    creature.update(0.5f);
    creature.set_direction(Direction::DOWN);
    creature.update(0.5f);
    EXPECT_EQ(creature.get_position(), (Position{0, 1}));
}

TEST_F(CreatureTest, HasSteppedOnlyOnTheTickItEntersACell) {
    creature.set_direction(Direction::DOWN);
    creature.update(1.0f);
    EXPECT_TRUE(creature.has_stepped());
    creature.update(0.5f);
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
    EXPECT_TRUE(location.get_entities().empty());
    EXPECT_EQ(other.get_entities().size(), 1u);
}

}  // namespace
