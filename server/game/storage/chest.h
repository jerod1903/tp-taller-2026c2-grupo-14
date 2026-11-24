#ifndef CHEST_H
#define CHEST_H

#include "server/consts.h"
#include "server/game/storage/inventory.h"

// Un cofre colocado en una Location: guarda su propio inventario de CHEST_SIZE slots.
class Chest {
public:
    Chest(): inventory(ServerConstants::CHEST_SIZE) {}

    Inventory& get_inventory() { return inventory; }
    const Inventory& get_inventory() const { return inventory; }
    ChestState state() const { return ChestState{true, inventory.get_state().slots}; }

private:
    Inventory inventory;
};

#endif  // CHEST_H
