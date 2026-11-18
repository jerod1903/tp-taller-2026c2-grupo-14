#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <string>

#include "common/game_dto.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/inventory.h"

class Player: public Creature {
public:
    Player(uint32_t id, const Position& pos, Location& location, std::string name, int health,
           float base_speed, int gold, int energy);

    const std::string& get_name() const;
    int get_gold() const;
    int get_energy() const;

    // Usa la herramienta equipada sobre `target`.
    void use_tool(const Position& target);

    void update(float dt) override;

    void select_inventory_slot(uint8_t slot);
    InventoryState get_inventory_state() const;
    // Si tiene el chat abierto: los demas le ven el globito de "escribiendo".
    void set_typing(bool typing);
    bool is_typing() const;
    // Agrega objetos a su inventario. Devuelve cuantos no entraron (0 si entraron todos).
    uint8_t receive(ItemType item, uint8_t quantity);
    // Lo cansa: hablar con un comerciante (y más adelante, cualquier acción). No baja de 0.
    void tire(int amount);
    // Lanza std::invalid_argument si no le alcanza: quien le vende se fija antes.
    void pay(int amount);
    void earn(int amount);
    int count(ItemType item) const;
    int room_for(ItemType item) const;
    // Lanza std::invalid_argument si no tiene tantos.
    void take(ItemType item, int quantity);

private:
    std::string name;
    int gold;
    int energy;
    Inventory inventory;
    bool typing = false;
};

#endif  // PLAYER_H
