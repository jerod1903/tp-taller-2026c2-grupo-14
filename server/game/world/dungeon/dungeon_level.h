#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/location.h"

// Las escaleras y la salida al pueblo son portales comunes, salen de las transiciones del mapa.
class DungeonLevel: public Location {
public:
    DungeonLevel(std::string name, Grid grid, int depth, uint32_t seed = std::random_device{}());

    int get_depth() const;

    // El nivel es dueño de sus enemigos y los registra como entidades de la Location.
    void spawn(std::unique_ptr<Enemy> enemy);

    // El enemigo vivo más cercano a `cell` que esté a menos de una celda de ella (se mueven entre
    // celdas, así que no tiene que estar justo ahí), o nullptr si no hay.
    Enemy* enemy_near(const Position& cell) const;

    const std::vector<std::unique_ptr<Enemy>>& get_enemies() const;

    // Los jugadores que están en el nivel ahora; sus enemigos ven siempre esta lista. Si estaba
    // vacío y llega alguien, los enemigos se reemplazan por otros en celdas al azar: cada vez que
    // se entra aparecen en otro lado.
    void set_visitors(std::vector<Creature*> players);

    // Actualiza sus entidades y da de baja a los enemigos que ya no vuelven.
    void update(float dt) override;

    // Para los EnemySpawner: dónde, con qué id y con qué semilla aparece cada enemigo.
    // Una celda libre y lejos de los portales que además cumpla `also`, si se lo pasa.
    std::optional<Position> spawn_cell(const std::function<bool(const Position&)>& also = nullptr);
    // Únicos entre todos los niveles.
    uint32_t new_enemy_id();
    uint32_t new_seed();
    const std::vector<Creature*>& get_visitors() const;

private:
    int depth;
    std::vector<std::unique_ptr<Enemy>> enemies;
    std::vector<Creature*> visitors;
    std::mt19937 rng;
    uint32_t next_enemy_id;

    // Saca a todos sus enemigos y pone los de un tipo sorteado.
    void repopulate();
    void remove_enemies();
    // Lejos de todos sus portales: el que entra (por la salida o la escalera) no los tiene encima.
    bool far_from_portals(const Position& pos) const;
    void remove_gone_enemies();
};

#endif  // DUNGEON_LEVEL_H
