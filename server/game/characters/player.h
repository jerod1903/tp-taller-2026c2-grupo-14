#ifndef PLAYER_H
#define PLAYER_H

#include <cstdint>
#include <optional>
#include <string>

#include "common/game_dto.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/inventory.h"
#include "server/game/world/spawn_point.h"
#include "server/persistence/saved_game.h"

class Enemy;

class Player: public Creature {
public:
    Player(uint32_t id, const Position& pos, Location& location, std::string name, int health,
           float base_speed, int gold, int energy);

    const std::string& get_name() const;
    int get_gold() const;
    int get_energy() const;
    // Le suma oro (un cheat; más adelante, vender).
    void add_gold(int amount);

    // Le resta energía, sin bajar de 0: la última acción se hace aunque no le alcance.
    void spend_energy(int amount);
    // Sin energía o sin vida cae inconsciente: no hace nada más hasta el día siguiente.
    bool is_unconscious() const;
    // Despierta en su cama: descansa (rest) y queda acostado en ella (lie_in).
    void wake_up(const SpawnPoint& bed, bool slept_in_bed);
    // Recupera toda su vida y energía si durmió en su cama, si no una fracción
    // (bad_night_recovery).
    void rest(bool slept_in_bed);
    // Queda acostado en su cama, quieto.
    void lie_in(const SpawnPoint& bed);
    // A mitad de velocidad cuando está cansado (tired_energy).
    float speed() const override;
    // Gasta lo que tiene en el slot elegido (una semilla al plantar).
    void remove_selected(uint8_t quantity);
    // Lo que tiene en el slot elegido del inventario.
    ItemType selected_item() const;

    // Con weak_health de su vida o menos: pega menos y, cerca de un vampiro muerto, lo revive.
    bool is_badly_hurt() const override;

    // Le pega con la espada si ya pasó el tiempo desde el último golpe. Si le pegó, las monedas que
    // soltó el enemigo con ese golpe (0 si no lo mató); vacío si todavía no podía pegar.
    std::optional<int> strike(Enemy& target);

    // Cuenta el tiempo entre golpes y camina.
    void update(float dt) override;

    void select_inventory_slot(uint8_t slot);
    InventoryState get_inventory_state() const;
    // Lo que se guarda de él: oro, vida, energía e inventario.
    SavedPlayer saved() const;
    void restore(const SavedPlayer& saved);
    // Si tiene el chat abierto: los demas le ven el globito de "escribiendo".
    void set_typing(bool typing);
    bool is_typing() const;
    // Agrega objetos a su inventario. Devuelve cuantos no entraron (0 si entraron todos).
    uint8_t receive(ItemType item, uint8_t quantity, ItemQuality quality = ItemQuality::Base);
    // Cuántos tiene de ese objeto, de cualquier calidad.
    int count(ItemType item) const;
    // Cuántos más de ese objeto, con calidad base, le entran en el inventario.
    int room_for(ItemType item) const;
    // Saca esa cantidad, de cualquier calidad. Lanza std::invalid_argument si no tiene tantos.
    void take(ItemType item, int quantity);

protected:
    // Gravemente herido pega menos (weak_damage de su daño).
    int attack_damage() const override;

private:
    std::string name;
    int gold;
    int energy;
    int max_health;
    int max_energy;
    Inventory inventory;
    bool typing = false;
    float attack_cooldown_left = 0;
};

#endif  // PLAYER_H
