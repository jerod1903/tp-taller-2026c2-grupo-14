#include "server/game/world/dungeon/spawners/orc_spawner.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <optional>
#include <random>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/orc.h"
#include "server/game/world/dungeon/dungeon_level.h"

namespace {
// Hasta cuántas celdas (en x y en y) del primero aparecen los demás
constexpr int GROUP_SPREAD = 2;
}  // namespace

void OrcSpawner::spawn(DungeonLevel& level) const {
    const ServerConfig::Orc& config = server_config().enemies.orc;
    std::mt19937 rng(level.new_seed());
    int size = std::uniform_int_distribution<int>(config.min_group, config.max_group)(rng);
    std::optional<Position> first = level.spawn_cell();
    if (!first) {
        return;
    }
    level.spawn(std::make_unique<Orc>(level.new_enemy_id(), *first, level, level.get_visitors(),
                                      level.new_seed()));
    for (int i = 1; i < size; ++i) {
        std::optional<Position> cell = level.spawn_cell([&first](const Position& pos) {
            return std::max(std::abs(pos.x - first->x), std::abs(pos.y - first->y)) <= GROUP_SPREAD;
        });
        if (!cell) {
            return;
        }
        level.spawn(std::make_unique<Orc>(level.new_enemy_id(), *cell, level, level.get_visitors(),
                                          level.new_seed()));
    }
}
