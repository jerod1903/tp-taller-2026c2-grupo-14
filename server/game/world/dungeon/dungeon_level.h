#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <memory>
#include <string>
#include <vector>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/location.h"

// Las escaleras y la salida al pueblo son portales comunes, salen de las transiciones del mapa.
class DungeonLevel: public Location {
public:
    DungeonLevel(std::string name, Grid grid, int depth);

    int get_depth() const;

    // El nivel es dueño de sus enemigos y los registra como entidades de la Location.
    void spawn(std::unique_ptr<Enemy> enemy);

    // Actualiza sus entidades y después da de baja a los enemigos muertos.
    void update(float dt) override;

private:
    int depth;
    std::vector<std::unique_ptr<Enemy>> enemies;

    void remove_dead_enemies();
};

#endif  // DUNGEON_LEVEL_H
