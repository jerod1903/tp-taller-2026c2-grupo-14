#include "server/game/world/dungeon/spawners/enemy_spawner.h"

#include <array>

#include "server/game/world/dungeon/spawners/orc_spawner.h"
#include "server/game/world/dungeon/spawners/skeleton_spawner.h"
#include "server/game/world/dungeon/spawners/skull_spawner.h"
#include "server/game/world/dungeon/spawners/vampire_spawner.h"

const EnemySpawner& random_spawner(std::mt19937& rng) {
    static const SkeletonSpawner skeletons;
    static const VampireSpawner vampires;
    static const OrcSpawner orcs;
    static const SkullSpawner skulls;
    static const std::array<const EnemySpawner*, 4> spawners = {&skeletons, &vampires, &orcs,
                                                                &skulls};
    std::uniform_int_distribution<std::size_t> choice(0, spawners.size() - 1);
    return *spawners[choice(rng)];
}
