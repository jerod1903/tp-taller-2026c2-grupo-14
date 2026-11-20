#include "server/game/characters/inventory.h"

#include <algorithm>
#include <stdexcept>
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

int Inventory::count(ItemType item) const {
    int total = 0;
    for (const InventorySlot& slot: slots) {
        if (slot.item == item) {
            total += slot.quantity;
        }
    }
    return total;
}

int Inventory::room_for(ItemType item) const {
    int max_stack = server_config().player.max_stack;
    int room = 0;
    for (const InventorySlot& slot: slots) {
        if (slot.item == item && slot.quality == ItemQuality::Base) {
            room += max_stack - slot.quantity;
        } else if (slot.item == ItemType::Empty) {
            room += max_stack;
        }
    }
    return room;
}

void Inventory::remove(ItemType item, int quantity) {
    if (count(item) < quantity) {
        throw std::invalid_argument("not enough items in the inventory");
    }
    for (auto slot = slots.rbegin(); slot != slots.rend() && quantity > 0; ++slot) {
        if (slot->item != item) {
            continue;
        }
        int taken = std::min<int>(quantity, slot->quantity);
        slot->quantity = static_cast<uint8_t>(slot->quantity - taken);
        quantity -= taken;
        if (slot->quantity == 0) {
            *slot = InventorySlot{ItemType::Empty, 0};
        }
    }
}

InventoryState Inventory::get_state() const {
    return InventoryState{selected_slot, std::vector<InventorySlot>(slots.begin(), slots.end())};
}

void Inventory::restore(const InventoryState& state) {
    slots = {};
    for (std::size_t i = 0; i < state.slots.size() && i < slots.size(); ++i) {
        slots[i] = state.slots[i];
    }
    select_slot(state.selectedSlot);
}
