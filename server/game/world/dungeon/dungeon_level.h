#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/location.h"
#include <map>

// Las escaleras y la salida al pueblo son portales comunes, salen de las transiciones del mapa.
class DungeonLevel: public Location {
public:
    DungeonLevel(std::string name, Grid grid, int depth, uint32_t seed = std::random_device{}());

    int get_depth() const;

    // El nivel es dueño de sus enemigos y los registra como entidades de la Location.
    void spawn(std::unique_ptr<Enemy> enemy);

    // Los jugadores que están en el nivel ahora; sus enemigos ven siempre esta lista. Si estaba
    // vacío y llega alguien, los enemigos se reemplazan por otros en celdas al azar: cada vez que
    // se entra aparecen en otro lado.
    void set_visitors(std::vector<Creature*> players);

    // Actualiza sus entidades y da de baja a los enemigos que ya no vuelven.
    void update(float dt) override;

    // Un mineral picable; hits_left son los golpes de pico que le faltan.
    struct OreNode {
        ItemType ore;
        int hits_left;
    };

    // Los minerales tapan el paso: se pican desde una celda de al lado.
    bool is_walkable(const Position& pos) const override;

    // Los minerales del nivel, uno por celda.
    const std::map<Position, OreNode>& get_ores() const;
    // nullptr si en esa celda no hay ninguno.
    OreNode* ore_at(const Position& pos);
    void remove_ore(const Position& pos);

    // Qué pasó con un golpe de pico.
    enum class Strike {
        NoOre,     // no había mineral en la celda
        Chipped,   // el mineral sigue en pie
        Crumbled,  // se rompió y no dio producto (lapislázuli)
        Mined,     // se rompió y dio su producto
    };
    struct StrikeResult {
        Strike strike;
        ItemType ore = ItemType::Empty;  // el producto, si strike == Mined
    };

    // Un golpe de pico en la celda. `roll` es un número en [0, 1) que decide las tiradas: el
    // lapislázuli se rompe si roll < lapis_break_chance. Sin `roll` se sortea.
    StrikeResult strike_ore(const Position& pos, float roll);
    StrikeResult strike_ore(const Position& pos);

private:
    int depth;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Creature*> visitors;
    std::mt19937 rng;
    uint32_t next_enemy_id;

    // Saca a todos sus enemigos y pone los de server_config.toml en celdas libres, lejos de los
    // portales.
    void repopulate();
    void remove_enemies();
    // Lejos de todos sus portales: el que entra (por la salida o la escalera) no los tiene encima.
    bool far_from_portals(const Position& pos) const;
    void remove_gone_enemies();

    std::map<Position, OreNode> ores;

    // Pone los minerales de server_config.toml en celdas libres, lejos de los portales. El oro solo
    // desde gold_min_depth.
    void populate_ores();
};

#endif  // DUNGEON_LEVEL_H
