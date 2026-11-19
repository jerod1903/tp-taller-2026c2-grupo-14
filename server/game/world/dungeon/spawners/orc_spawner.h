#ifndef ORC_SPAWNER_H
#define ORC_SPAWNER_H

#include "server/game/world/dungeon/spawners/enemy_spawner.h"

// Los orcos aparecen en grupo: entre min_group y max_group de [enemies.orc]. El primero cae en una
// celda al azar y los demás a GROUP_SPREAD celdas o menos de él. Si no hay lugar, pone los que
// entren.
class OrcSpawner: public EnemySpawner {
public:
    void spawn(DungeonLevel& level) const override;
};

#endif  // ORC_SPAWNER_H
