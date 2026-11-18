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

    // Apila primero sobre los slots que ya tienen ese objeto (hasta MAX_STACK) y despues usa los
    // vacios. Devuelve cuantos NO entraron: 0 si entro todo.
    uint8_t add(ItemType item, uint8_t quantity);

    // Cuántos hay de ese objeto, sumando todos los slots.
    int count(ItemType item) const;
    // Cuántos más de ese objeto entran: lo que falta para llenar sus pilas más los slots vacíos.
    int room_for(ItemType item) const;
    // Saca esa cantidad, empezando por los últimos slots; el que queda en 0 se vacía. Lanza
    // std::invalid_argument si no hay tantos.
    void remove(ItemType item, int quantity);

    InventoryState get_state() const;

private:
    std::array<InventorySlot, INVENTORY_SIZE> slots{};
    uint8_t selected_slot = 0;
};

#endif  // INVENTORY_H
