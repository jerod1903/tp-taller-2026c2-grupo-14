#include "server/game/storage/inventory.h"

#include <algorithm>
#include <stdexcept>
#include <utility>
#include <vector>

#include "server/config/server_config.h"

Inventory::Inventory(uint8_t size, std::vector<InventorySlot> initial_items):
        slots(std::move(initial_items)) {
    if (slots.size() > size) {
        throw std::invalid_argument("too many initial items for the inventory");
    }
    slots.resize(size);
}

void Inventory::select_slot(uint8_t slot) {
    if (slot < slots.size()) {
        selected_slot = slot;
    }
}

uint8_t Inventory::add(ItemType item, uint8_t quantity, ItemQuality quality) {
    // Primero completa las pilas que ya hay de ese objeto con esa calidad
    for (InventorySlot& slot: slots) {
        if (quantity > 0 && slot.item == item && slot.quality == quality &&
            slot.quantity < server_config().inventory.max_stack) {
            uint8_t added = std::min<uint8_t>(quantity,
                                              server_config().inventory.max_stack - slot.quantity);
            slot.quantity += added;
            quantity -= added;
        }
    }
    // Lo que sobra va a slots vacios, de a una pila por slot
    for (InventorySlot& slot: slots) {
        if (quantity > 0 && slot.item == ItemType::Empty) {
            uint8_t added = std::min<uint8_t>(quantity, server_config().inventory.max_stack);
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

ItemType Inventory::item_at(uint8_t slot) const {
    return slot < slots.size() ? slots[slot].item : ItemType::Empty;
}

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
    int max_stack = server_config().inventory.max_stack;
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

InventoryState Inventory::get_state() const { return InventoryState{selected_slot, slots}; }

void Inventory::restore(const InventoryState& state) {
    std::fill(slots.begin(), slots.end(), InventorySlot{ItemType::Empty, 0});
    for (std::size_t i = 0; i < state.slots.size() && i < slots.size(); ++i) {
        slots[i] = state.slots[i];
    }
    select_slot(state.selectedSlot);
}

uint8_t Inventory::move_slot(uint8_t slot, Inventory& target) {
    if (slot >= slots.size() || slots[slot].item == ItemType::Empty) {
        return 0;
    }
    InventorySlot& source = slots[slot];
    uint8_t left = target.add(source.item, source.quantity, source.quality);
    uint8_t moved = source.quantity - left;
    source.quantity = left;
    if (left == 0) {
        source = InventorySlot{ItemType::Empty, 0};
    }
    return moved;
}


bool Inventory::split_slot(uint8_t slot) {
    if (slot >= slots.size() || slots[slot].quantity < 2) {
        return false;
    }
    auto empty = std::find_if(slots.begin(), slots.end(), [](const InventorySlot& candidate) {
        return candidate.item == ItemType::Empty;
    });
    if (empty == slots.end()) {
        return false;
    }
    InventorySlot& source = slots[slot];
    uint8_t half = source.quantity / 2;
    *empty = InventorySlot{source.item, half, source.quality};
    source.quantity -= half;
    return true;
}

int Inventory::count(ItemType item, ItemQuality quality) const {
    int total = 0;
    for (const InventorySlot& slot: slots) {
        if (slot.item == item && slot.quality == quality) {
            total += slot.quantity;
        }
    }
    return total;
}

void Inventory::remove(ItemType item, ItemQuality quality, int quantity) {
    if (count(item, quality) < quantity) {
        throw std::invalid_argument("not enough items of that quality in the inventory");
    }
    for (auto slot = slots.rbegin(); slot != slots.rend() && quantity > 0; ++slot) {
        if (slot->item != item || slot->quality != quality) {
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

bool Inventory::is_empty() const {
    return std::all_of(slots.begin(), slots.end(),
                       [](const InventorySlot& s) { return s.item == ItemType::Empty; });
}