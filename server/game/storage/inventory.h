#ifndef INVENTORY_H
#define INVENTORY_H

#include <cstdint>
#include <map>
#include <vector>

#include "common/game_dto.h"

class Inventory {
public:
    // `size` slots con `initial_items` en los primeros, en orden; el resto vacío. Lanza
    // std::invalid_argument si hay más items que slots.
    explicit Inventory(uint8_t size, std::vector<InventorySlot> initial_items = {});

    // Un slot fuera de rango se ignora.
    void select_slot(uint8_t slot);

    // Apila primero sobre los slots que ya tienen ese objeto (hasta max_stack) y despues usa los
    // vacios. Devuelve cuantos NO entraron: 0 si entro todo.
    uint8_t add(ItemType item, uint8_t quantity, ItemQuality quality = ItemQuality::Base);

    // Saca hasta `quantity` del slot elegido; si queda en 0, el slot queda vacío.
    void remove_selected(uint8_t quantity);

    // Lo que hay en el slot elegido (ItemType::Empty si está vacío).
    ItemType selected_item() const;
    // Lo que hay en ese slot (ItemType::Empty si está vacío o fuera de rango).
    ItemType item_at(uint8_t slot) const;

    // Cuántos hay de ese objeto, sumando todos los slots.
    int count(ItemType item) const;
    // Cuántos más de ese objeto entran: lo que falta para llenar sus pilas más los slots vacíos.
    int room_for(ItemType item) const;
    // Saca esa cantidad, empezando por los últimos slots; el que queda en 0 se vacía. Devuelve
    // cuántos sacó de cada calidad. Lanza std::invalid_argument si no hay tantos.
    std::map<ItemQuality, int> remove(ItemType item, int quantity);

    InventoryState get_state() const;
    // Vuelve a como estaba en `state` (uno guardado); los slots que falten quedan vacíos.
    void restore(const InventoryState& state);

    // Pasa la pila de `slot` a `target` (apila primero sobre lo que ya hay); lo que no entra queda
    // en el slot. Devuelve cuántos pasaron: 0 si el slot está vacío o fuera de rango.
    uint8_t move_slot(uint8_t slot, Inventory& target);
    // Pasa la mitad de la pila de `slot` al primer slot vacío; con impar, la mitad mayor queda.
    // false si el slot está fuera de rango, tiene menos de 2 o no hay ningún slot vacío.
    bool split_slot(uint8_t slot);


private:
    std::vector<InventorySlot> slots;
    uint8_t selected_slot = 0;
};

#endif  // INVENTORY_H
