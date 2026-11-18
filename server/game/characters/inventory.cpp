#include "server/game/characters/inventory.h"

#include <algorithm>
#include <vector>

#include "server/config/server_config.h"

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

uint8_t Inventory::add(ItemType item, uint8_t quantity, ItemQuality quality) {
    // Primero completa las pilas que ya hay de ese objeto con esa calidad
    for (InventorySlot& slot: slots) {
        if (quantity > 0 && slot.item == item && slot.quality == quality &&
            slot.quantity < server_config().player.max_stack) {
            uint8_t added =
                    std::min<uint8_t>(quantity, server_config().player.max_stack - slot.quantity);
            slot.quantity += added;
            quantity -= added;
        }
    }
    // Lo que sobra va a slots vacios, de a una pila por slot
    for (InventorySlot& slot: slots) {
        if (quantity > 0 && slot.item == ItemType::Empty) {
            uint8_t added = std::min<uint8_t>(quantity, server_config().player.max_stack);
            slot = InventorySlot{item, added, quality};
            quantity -= added;
        }
    }
    return quantity;
}

void Inventory::remove_selected(uint8_t quantity) {
    InventorySlot& slot = slots[selected_slot];
    slot.quantity -= std::min(quantity, slot.quantity);
    if (slot.quantity == 0) {
        slot = InventorySlot{ItemType::Empty, 0};
    }
}

ItemType Inventory::selected_item() const { return slots[selected_slot].item; }

InventoryState Inventory::get_state() const {
    return InventoryState{selected_slot, std::vector<InventorySlot>(slots.begin(), slots.end())};
}
