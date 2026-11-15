#include <cstddef>

#include <gtest/gtest.h>

#include "server/game/characters/inventory.h"

TEST(InventoryTest, StartsWithTheFourToolsAndTheRestEmpty) {
    Inventory inventory;
    InventoryState state = inventory.get_state();
    ASSERT_EQ(state.slots.size(), static_cast<std::size_t>(INVENTORY_SIZE));
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
    Inventory inventory;
    EXPECT_EQ(inventory.get_state().selectedSlot, 0);
}

TEST(InventoryTest, SelectSlotChangesTheSelectedSlot) {
    Inventory inventory;
    inventory.select_slot(2);
    EXPECT_EQ(inventory.get_state().selectedSlot, 2);
}

TEST(InventoryTest, SelectSlotOutOfRangeIsIgnored) {
    Inventory inventory;
    inventory.select_slot(2);
    inventory.select_slot(INVENTORY_SIZE);
    inventory.select_slot(200);
    EXPECT_EQ(inventory.get_state().selectedSlot, 2);
}
TEST(InventoryTest, AddPutsANewItemInTheFirstEmptySlot) {
    Inventory inventory;
    EXPECT_EQ(inventory.add(ItemType::Milk, 3), 0);
    InventoryState state = inventory.get_state();
    EXPECT_EQ(state.slots[4], (InventorySlot{ItemType::Milk, 3}));
}

TEST(InventoryTest, AddStacksOnTheSameItemUpToMaxStack) {
    Inventory inventory;
    inventory.add(ItemType::Milk, 45);
    EXPECT_EQ(inventory.add(ItemType::Milk, 10), 0);
    InventoryState state = inventory.get_state();
    EXPECT_EQ(state.slots[4], (InventorySlot{ItemType::Milk, 50}));
    EXPECT_EQ(state.slots[5], (InventorySlot{ItemType::Milk, 5}));
}

TEST(InventoryTest, AddReturnsWhatDoesNotFit) {
    Inventory inventory;
    // 4 herramientas + 5 slots libres de 50 leches: entran 250
    EXPECT_EQ(inventory.add(ItemType::Milk, 250), 0);
    EXPECT_EQ(inventory.add(ItemType::Milk, 1), 1);
}
