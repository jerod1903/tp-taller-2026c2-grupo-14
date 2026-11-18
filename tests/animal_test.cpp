#include "server/game/characters/animals/animal.h"

#include "server/game/characters/animals/chicken/chicken.h"
#include "server/game/characters/animals/cow/cow.h"

#include <set>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "tests/world_helpers.h"

namespace {

constexpr uint32_t SEED = 7;
const float TICK = 1.0f / server_config().game.tick_rate;

}  // namespace

TEST(AnimalTest, KeepsItsKind) {
    TestLocation location;
    Cow cow(0, Position{2, 2}, location, SEED);
    EXPECT_EQ(cow.get_kind(), AnimalKind::Cow);
}

TEST(AnimalTest, ChangesDirectionOverTime) {
    TestLocation location;
    Chicken chicken(0, Position{2, 2}, location, SEED);
    std::set<Direction> seen;
    for (int i = 0; i < 60 * server_config().game.tick_rate; ++i) {  // un minuto
        chicken.update(TICK);
        seen.insert(chicken.get_direction());
    }
    EXPECT_GT(seen.size(), 2u);
}

TEST(AnimalTest, NeverLeavesAWalledPen) {
    // Corral de 3x3 rodeado de paredes, en una grilla de 5x5
    TestLocation location{make_grid(5, 5,
                                    {{0, 0},
                                     {1, 0},
                                     {2, 0},
                                     {3, 0},
                                     {4, 0},
                                     {0, 4},
                                     {1, 4},
                                     {2, 4},
                                     {3, 4},
                                     {4, 4},
                                     {0, 1},
                                     {0, 2},
                                     {0, 3},
                                     {4, 1},
                                     {4, 2},
                                     {4, 3}})};
    Cow cow(0, Position{2, 2}, location, SEED);
    for (int i = 0; i < 60 * server_config().game.tick_rate; ++i) {
        cow.update(TICK);
        ASSERT_GE(cow.get_position().x, 1);
        ASSERT_LE(cow.get_position().x, 3);
        ASSERT_GE(cow.get_position().y, 1);
        ASSERT_LE(cow.get_position().y, 3);
    }
}

TEST(AnimalTest, ACowIsReadyAfterItsProductionDays) {
    TestLocation location;
    Cow cow(0, Position{2, 2}, location, SEED);
    for (int day = 1; day < server_config().animals.cow.production_days; ++day) {
        cow.start_new_day();
    }
    EXPECT_FALSE(cow.is_ready());
    cow.start_new_day();
    EXPECT_TRUE(cow.is_ready());
}

TEST(AnimalTest, MilkingStartsTheCountAgain) {
    TestLocation location;
    Cow cow(0, Position{2, 2}, location, SEED);
    for (int day = 0; day < server_config().animals.cow.production_days; ++day) {
        cow.start_new_day();
    }
    cow.milk();
    EXPECT_FALSE(cow.is_ready());
    for (int day = 0; day < server_config().animals.cow.production_days; ++day) {
        cow.start_new_day();
    }
    EXPECT_TRUE(cow.is_ready());
}

TEST(AnimalTest, AChickenIsNeverReady) {
    TestLocation location;
    Chicken chicken(0, Position{2, 2}, location, SEED);
    for (int day = 0; day < 10; ++day) {
        chicken.start_new_day();
    }
    EXPECT_FALSE(chicken.is_ready());
}
