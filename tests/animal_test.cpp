#include "server/game/characters/animals/animal.h"

#include <set>

#include <gtest/gtest.h>

#include "server/consts.h"
#include "tests/world_helpers.h"

namespace {

constexpr uint32_t SEED = 7;
constexpr float TICK = 1.0f / ServerConstants::TICK_RATE;

}  // namespace

TEST(AnimalTest, KeepsItsKind) {
    TestLocation location;
    Animal cow(0, AnimalKind::Cow, Position{2, 2}, location, SEED);
    EXPECT_EQ(cow.get_kind(), AnimalKind::Cow);
}

TEST(AnimalTest, ChangesDirectionOverTime) {
    TestLocation location;
    Animal chicken(0, AnimalKind::Chicken, Position{2, 2}, location, SEED);
    std::set<Direction> seen;
    for (int i = 0; i < 60 * ServerConstants::TICK_RATE; ++i) {  // un minuto
        chicken.update(TICK);
        seen.insert(chicken.get_direction());
    }
    EXPECT_GT(seen.size(), 2u);
}

TEST(AnimalTest, NeverLeavesAWalledPen) {
    // Corral de 3x3 rodeado de paredes, en una grilla de 5x5
    TestLocation location{make_grid(5, 5, {{0, 0}, {1, 0}, {2, 0}, {3, 0}, {4, 0}, {0, 4},
                                           {1, 4}, {2, 4}, {3, 4}, {4, 4}, {0, 1}, {0, 2},
                                           {0, 3}, {4, 1}, {4, 2}, {4, 3}})};
    Animal cow(0, AnimalKind::Cow, Position{2, 2}, location, SEED);
    for (int i = 0; i < 60 * ServerConstants::TICK_RATE; ++i) {
        cow.update(TICK);
        ASSERT_GE(cow.get_position().x, 1);
        ASSERT_LE(cow.get_position().x, 3);
        ASSERT_GE(cow.get_position().y, 1);
        ASSERT_LE(cow.get_position().y, 3);
    }
}
