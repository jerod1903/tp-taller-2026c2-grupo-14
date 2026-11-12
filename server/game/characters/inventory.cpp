#include "server/game/characters/inventory.h"

#include <vector>

Inventory::Inventory() {
    slots[0] = {ItemType::Pickaxe, 1};
    slots[1] = {ItemType::WateringCan, 1};
    slots[2] = {ItemType::Shovel, 1};
    slots[3] = {ItemType::Sword, 1};
}

void Inventory::select_slot(uint8_t slot) {
    if (slot < INVENTORY_SIZE) {
        selected_slot = slot;
    }
}

InventoryState Inventory::get_state() const {
    return InventoryState{selected_slot, std::vector<InventorySlot>(slots.begin(), slots.end())};
}