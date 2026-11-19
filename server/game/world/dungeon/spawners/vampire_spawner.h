#ifndef VAMPIRE_SPAWNER_H
#define VAMPIRE_SPAWNER_H

#include "server/game/world/dungeon/spawners/enemy_spawner.h"

// Los vampiros aparecen en pareja, un vampiro y una vampira en dos celdas pegadas: `pairs` de
// [enemies.vampire]. Si no hay lugar para los dos, no pone ninguno.
class VampireSpawner: public EnemySpawner {
public:
    void spawn(DungeonLevel& level) const override;

private:
    void spawn_pair(DungeonLevel& level) const;
};

#endif  // VAMPIRE_SPAWNER_H
