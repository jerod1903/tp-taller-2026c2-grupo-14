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

TEST_F(PlayerTest, TakeRemovesFromItsInventory) {
    player.receive(ItemType::Milk, 4);
    player.take(ItemType::Milk, 3);
    EXPECT_EQ(player.count(ItemType::Milk), 1);
}

TEST_F(PlayerTest, TakeRemovesAnyQuality) {
    player.receive(ItemType::Potato, 2, ItemQuality::Perfect);
    player.receive(ItemType::Potato, 1);
    player.take(ItemType::Potato, 3);
    EXPECT_EQ(player.count(ItemType::Potato), 0);
}

TEST_F(PlayerTest, TakingMoreThanItHasThrows) {
    EXPECT_THROW(player.take(ItemType::Milk, 1), std::invalid_argument);
}

TEST_F(PlayerTest, RoomForComesFromItsInventory) {
    Inventory empty;
    EXPECT_EQ(player.room_for(ItemType::Milk), empty.room_for(ItemType::Milk));
}
