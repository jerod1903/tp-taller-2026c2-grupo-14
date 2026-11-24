#include "server/game/storage/inventory.h"

#include <cstddef>
#include <stdexcept>
#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/consts.h"

namespace {

const std::vector<InventorySlot> TOOLS = {{ItemType::Pickaxe, 1},
                                          {ItemType::WateringCan, 1},
                                          {ItemType::Shovel, 1},
                                          {ItemType::Sword, 1}};

Inventory make_inventory() { return Inventory(ServerConstants::PLAYER_INVENTORY_SIZE, TOOLS); }

}  // namespace

TEST(InventoryTest, StartsWithTheInitialItemsAndTheRestEmpty) {
    Inventory inventory = make_inventory();
    InventoryState state = inventory.get_state();
    ASSERT_EQ(state.slots.size(), static_cast<std::size_t>(ServerConstants::PLAYER_INVENTORY_SIZE));
    EXPECT_EQ(state.slots[0].item, ItemType::Pickaxe);
    EXPECT_EQ(state.slots[1].item, ItemType::WateringCan);
    EXPECT_EQ(state.slots[2].item, ItemType::Shovel);
    EXPECT_EQ(state.slots[3].item, ItemType::Sword);
    for (std::size_t i = 4; i < state.slots.size(); ++i) {
        EXPECT_EQ(state.slots[i].item, ItemType::Empty);
        EXPECT_EQ(state.slots[i].quantity, 0);
    }
}

TEST(InventoryTest, StartsWithTheFirstSlotSelected) {
    Inventory inventory = make_inventory();
    EXPECT_EQ(inventory.get_state().selectedSlot, 0);
}

TEST(InventoryTest, SelectSlotChangesTheSelectedSlot) {
    Inventory inventory = make_inventory();
    inventory.select_slot(2);
    EXPECT_EQ(inventory.get_state().selectedSlot, 2);
}

TEST(InventoryTest, SelectSlotOutOfRangeIsIgnored) {
    Inventory inventory = make_inventory();
    inventory.select_slot(2);
    inventory.select_slot(ServerConstants::PLAYER_INVENTORY_SIZE);
    inventory.select_slot(200);
    EXPECT_EQ(inventory.get_state().selectedSlot, 2);
}
TEST(InventoryTest, AddPutsANewItemInTheFirstEmptySlot) {
    Inventory inventory = make_inventory();
    EXPECT_EQ(inventory.add(ItemType::Milk, 3), 0);
    InventoryState state = inventory.get_state();
    EXPECT_EQ(state.slots[4], (InventorySlot{ItemType::Milk, 3}));
}

TEST(InventoryTest, AddStacksOnTheSameItemUpToMaxStack) {
    Inventory inventory = make_inventory();
    inventory.add(ItemType::Milk, 45);
    EXPECT_EQ(inventory.add(ItemType::Milk, 10), 0);
    InventoryState state = inventory.get_state();
    EXPECT_EQ(state.slots[4], (InventorySlot{ItemType::Milk, 50}));
    EXPECT_EQ(state.slots[5], (InventorySlot{ItemType::Milk, 5}));
}

TEST(InventoryTest, AddReturnsWhatDoesNotFit) {
    Inventory inventory = make_inventory();
    // 4 herramientas + 5 slots libres de 50 leches: entran 250
    EXPECT_EQ(inventory.add(ItemType::Milk, 250), 0);
    EXPECT_EQ(inventory.add(ItemType::Milk, 1), 1);
}

TEST(InventoryTest, CountAddsUpEverySlotOfThatItem) {
    Inventory inventory = make_inventory();
    inventory.add(ItemType::Milk, 60);
    EXPECT_EQ(inventory.count(ItemType::Milk), 60);
}

TEST(InventoryTest, RoomForCountsTheStacksAndTheEmptySlots) {
    Inventory inventory = make_inventory();
    inventory.add(ItemType::Milk, 10);
    int max_stack = server_config().inventory.max_stack;
    // Las 4 herramientas ocupan 4 slots: quedan 5, uno con 10 de leche
    EXPECT_EQ(inventory.room_for(ItemType::Milk), 5 * max_stack - 10);
}

TEST(InventoryTest, RemoveTakesFromTheLastSlotsFirst) {
    Inventory inventory = make_inventory();
    int max_stack = server_config().inventory.max_stack;
    inventory.add(ItemType::Milk, static_cast<uint8_t>(max_stack + 5));
    inventory.remove(ItemType::Milk, 5);
    EXPECT_EQ(inventory.get_state().slots[4],
              (InventorySlot{ItemType::Milk, static_cast<uint8_t>(max_stack)}));
    EXPECT_EQ(inventory.get_state().slots[5], (InventorySlot{ItemType::Empty, 0}));
}

TEST(InventoryTest, RemovingEverythingLeavesTheSlotEmpty) {
    Inventory inventory = make_inventory();
    inventory.add(ItemType::Milk, 3);
    inventory.remove(ItemType::Milk, 3);
    EXPECT_EQ(inventory.count(ItemType::Milk), 0);
    EXPECT_EQ(inventory.get_state().slots[4], (InventorySlot{ItemType::Empty, 0}));
}

TEST(InventoryTest, RemovingMoreThanThereIsThrowsAndRemovesNothing) {
    Inventory inventory = make_inventory();
    inventory.add(ItemType::Milk, 2);
    EXPECT_THROW(inventory.remove(ItemType::Milk, 3), std::invalid_argument);
    EXPECT_EQ(inventory.count(ItemType::Milk), 2);
}

TEST(InventoryQuality, CountsOnlyThatQuality) {
    Inventory inv = make_inventory();
    inv.add(ItemType::Potato, 3, ItemQuality::Base);
    inv.add(ItemType::Potato, 2, ItemQuality::Perfect);
    EXPECT_EQ(inv.count(ItemType::Potato, ItemQuality::Perfect), 2);
    EXPECT_EQ(inv.count(ItemType::Potato, ItemQuality::Base), 3);
    EXPECT_EQ(inv.count(ItemType::Potato, ItemQuality::Incredible), 0);
}

TEST(InventoryQuality, RemoveLeavesOtherQualitiesUntouched) {
    Inventory inv = make_inventory();
    inv.add(ItemType::Potato, 3, ItemQuality::Base);
    inv.add(ItemType::Potato, 2, ItemQuality::Perfect);
    inv.remove(ItemType::Potato, ItemQuality::Perfect, 2);
    EXPECT_EQ(inv.count(ItemType::Potato, ItemQuality::Perfect), 0);
    EXPECT_EQ(inv.count(ItemType::Potato, ItemQuality::Base), 3);
}

TEST(InventoryQuality, RemoveMoreThanThereIsThrowsAndChangesNothing) {
    Inventory inv = make_inventory();
    inv.add(ItemType::Potato, 1, ItemQuality::Perfect);
    EXPECT_THROW(inv.remove(ItemType::Potato, ItemQuality::Perfect, 2), std::invalid_argument);
    EXPECT_EQ(inv.count(ItemType::Potato, ItemQuality::Perfect), 1);
}

TEST(InventoryQuality, IsEmpty) {
    Inventory inv(ServerConstants::PLAYER_INVENTORY_SIZE);
    EXPECT_TRUE(inv.is_empty());
    inv.add(ItemType::Egg, 1);
    EXPECT_FALSE(inv.is_empty());
    inv.remove(ItemType::Egg, 1);
    EXPECT_TRUE(inv.is_empty());
}