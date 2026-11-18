#include "server/game/characters/player.h"

#include <stdexcept>

#include <gtest/gtest.h>

#include "tests/world_helpers.h"

namespace {

constexpr int GOLD = 100;
constexpr int ENERGY = 50;

class PlayerTest: public ::testing::Test {
protected:
    TestLocation location;
    Player player{1, Position{0, 0}, location, "jero", 100, 1.0f, GOLD, ENERGY};
};

}  // namespace

TEST_F(PlayerTest, TireLowersTheEnergy) {
    player.tire(10);
    EXPECT_EQ(player.get_energy(), ENERGY - 10);
}

TEST_F(PlayerTest, TheEnergyDoesNotGoBelowZero) {
    player.tire(ENERGY + 1);
    EXPECT_EQ(player.get_energy(), 0);
}

TEST_F(PlayerTest, PayAndEarnChangeTheGold) {
    player.pay(30);
    player.earn(5);
    EXPECT_EQ(player.get_gold(), GOLD - 30 + 5);
}

TEST_F(PlayerTest, PayingMoreThanItHasThrowsAndKeepsTheGold) {
    EXPECT_THROW(player.pay(GOLD + 1), std::invalid_argument);
    EXPECT_EQ(player.get_gold(), GOLD);
}

TEST_F(PlayerTest, TakeRemovesFromItsInventory) {
    player.receive(ItemType::Milk, 4);
    player.take(ItemType::Milk, 3);
    EXPECT_EQ(player.count(ItemType::Milk), 1);
}

TEST_F(PlayerTest, RoomForComesFromItsInventory) {
    Inventory empty;
    EXPECT_EQ(player.room_for(ItemType::Milk), empty.room_for(ItemType::Milk));
}
