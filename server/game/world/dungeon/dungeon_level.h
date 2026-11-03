#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <memory>
#include <string>
#include <vector>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/location.h"
#include "server/game/world/portal.h"
#include "server/game/world/position.h"

class DungeonLevel: public Location {
public:
    DungeonLevel(std::string name, Grid grid, int depth, const Position& stairs_down);

    int get_depth() const;
    const Position& get_stairs_down() const;

    // La salida al pueblo. Se fija al enlazar el mundo, cuando el Town ya existe.
    void set_exit_portal(const Portal& portal);
    const Portal& get_exit_portal() const;

    // El nivel es dueño de sus enemigos y los registra como entidades de la Location.
    void spawn(std::unique_ptr<Enemy> enemy);

    // Actualiza sus entidades y después da de baja a los enemigos muertos.
    void update(float dt) override;

private:
    int depth;
    Portal exit_portal;
    Position stairs_down;
    std::vector<std::unique_ptr<Enemy>> enemies;

    void remove_dead_enemies();
};

#endif  // DUNGEON_LEVEL_H
