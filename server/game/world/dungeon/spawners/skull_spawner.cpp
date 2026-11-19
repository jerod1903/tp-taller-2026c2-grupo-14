#include "server/game/world/dungeon/spawners/skull_spawner.h"

#include <memory>
#include <optional>
#include <random>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/skull.h"
#include "server/game/world/dungeon/dungeon_level.h"

void SkullSpawner::spawn(DungeonLevel& level) const {
    std::optional<Position> cell = level.spawn_cell();
    if (!cell) {
        return;
    }
    std::mt19937 rng(level.new_seed());
    bool green = std::bernoulli_distribution(server_config().enemies.skull.green_chance)(rng);
    level.spawn(std::make_unique<Skull>(level.new_enemy_id(), *cell, level, level.get_visitors(),
                                        green ? FlameKind::Green : FlameKind::Blue,
                                        level.new_seed()));
}
