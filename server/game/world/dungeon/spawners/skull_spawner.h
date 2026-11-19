#ifndef SKULL_SPAWNER_H
#define SKULL_SPAWNER_H

#include "server/game/world/dungeon/spawners/enemy_spawner.h"

// La calavera aparece sola, y hay una sola por nivel, en una celda al azar. Sus llamas son verdes
// con probabilidad green_chance de [enemies.skull]; si no, azules.
class SkullSpawner: public EnemySpawner {
public:
    void spawn(DungeonLevel& level) const override;
};

#endif  // SKULL_SPAWNER_H
