#include "server/game/world/dungeon/spawners/skeleton_spawner.h"

#include <memory>
#include <optional>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/skeleton.h"
#include "server/game/world/dungeon/dungeon_level.h"

void SkeletonSpawner::spawn(DungeonLevel& level) const {
    for (int i = 0; i < server_config().enemies.skeleton.count; ++i) {
        std::optional<Position> cell = level.spawn_cell();
        if (!cell) {
            return;
        }
        level.spawn(std::make_unique<Skeleton>(level.new_enemy_id(), *cell, level,
                                               level.get_visitors(), level.new_seed()));
    }
}
