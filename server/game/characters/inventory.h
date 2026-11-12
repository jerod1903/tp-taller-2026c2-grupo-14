#ifndef INVENTORY_H
#define INVENTORY_H

#include <array>
#include <cstdint>

#include "common/game_dto.h"

class Inventory {
public:
    // Las herramientas y la cama en los primeros slots; el resto vacío.
    Inventory();

    // Un slot fuera de rango se ignora.
    void select_slot(uint8_t slot);

    InventoryState get_state() const;

private:
    std::array<InventorySlot, INVENTORY_SIZE> slots{};
    uint8_t selected_slot = 0;
};

#endif  // INVENTORY_H