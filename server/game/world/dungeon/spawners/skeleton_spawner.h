#ifndef SKELETON_SPAWNER_H
#define SKELETON_SPAWNER_H

#include "server/game/world/dungeon/spawners/enemy_spawner.h"

// Los esqueletos aparecen solos: `count` de [enemies.skeleton], cada uno en una celda al azar.
class SkeletonSpawner: public EnemySpawner {
public:
    void spawn(DungeonLevel& level) const override;
};

#endif  // SKELETON_SPAWNER_H
