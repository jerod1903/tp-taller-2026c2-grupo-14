#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/dungeon/traps/trap.h"
#include "server/game/world/location.h"

// Las escaleras y la salida al pueblo son portales comunes, salen de las transiciones del mapa.
class Flame;

class DungeonLevel: public Location {
public:
    DungeonLevel(std::string name, Grid grid, int depth, uint32_t seed = std::random_device{}());

    int get_depth() const;

    // El nivel es dueño de sus enemigos y sus trampas y los registra como entidades de la
    // Location.
    void spawn(std::unique_ptr<Enemy> enemy);
    void spawn(std::unique_ptr<Trap> trap);

    // El enemigo vivo más cercano a `cell` que esté a menos de una celda de ella (se mueven entre
    // celdas, así que no tiene que estar justo ahí), o nullptr si no hay.
    Enemy* enemy_near(const Position& cell) const;

    const std::vector<std::unique_ptr<Enemy>>& get_enemies() const;
    const std::vector<std::unique_ptr<Trap>>& get_traps() const;

    // Los jugadores que están en el nivel ahora; sus enemigos ven siempre esta lista. Si estaba
    // vacío y llega alguien, los enemigos y las trampas se reemplazan por otros en celdas al azar:
    // cada vez que se entra aparecen en otro lado.
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
        bool spawn_orcs = false;         // true si salió plata y se avecina una manada de orcos
    };

    // Un golpe de pico en la celda. `roll` es un número en [0, 1) que decide las tiradas: el
    // lapislázuli se rompe si roll < lapis_break_chance. Sin `roll` se sortea.
    StrikeResult strike_ore(const Position& pos, float roll);
    StrikeResult strike_ore(const Position& pos);
    // Para los EnemySpawner y el TrapSpawner: dónde, con qué id y con qué semilla aparece cada
    // enemigo. Una celda libre y lejos de los portales que además cumpla `also`, si se lo pasa.
    std::optional<Position> spawn_cell(const std::function<bool(const Position&)>& also = nullptr);
    // No se repite entre los niveles, aunque vuelve a empezar después de muchos.
    uint32_t new_enemy_id();
    uint32_t new_seed();
    const std::vector<Creature*>& get_visitors() const;

    // Las llamas que andan por el nivel, para el snapshot. No es su dueño: cada llama se anota al
    // crearse y se borra al destruirse.
    void add_flame(const Flame& flame);
    void remove_flame(const Flame& flame);
    const std::vector<const Flame*>& get_flames() const;
    // Para las llamas: no se repite entre los niveles, aunque vuelve a empezar después de muchas.
    uint32_t new_flame_id();

private:
    int depth;
    // Antes que `enemies`: se destruye después, así las llamas de los enemigos se pueden borrar
    // de acá al destruirse.
    std::vector<const Flame*> flames;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<std::unique_ptr<Trap>> traps;
    std::vector<Creature*> visitors;
    std::mt19937 rng;
    uint32_t enemies_made = 0;
    uint32_t flames_made = 0;

    // Saca a todos sus enemigos y sus trampas, y pone trampas nuevas y enemigos de un tipo
    // sorteado.
    void repopulate();
    void remove_enemies();
    void remove_traps();
    // Lejos de todos sus portales: el que entra (por la salida o la escalera) no los tiene encima.
    bool far_from_portals(const Position& pos) const;
    void remove_gone_enemies();

    std::map<Position, OreNode> ores;

    // Pone los minerales de server_config.toml en celdas libres, lejos de los portales. El oro solo
    // desde gold_min_depth.
    void populate_ores();
};

#endif  // DUNGEON_LEVEL_H
