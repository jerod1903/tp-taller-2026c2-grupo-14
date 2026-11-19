#include "server/game/world/dungeon/spawners/vampire_spawner.h"

#include <cstdlib>
#include <memory>
#include <optional>
#include <utility>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/vampire.h"
#include "server/game/world/dungeon/dungeon_level.h"

void VampireSpawner::spawn(DungeonLevel& level) const {
    for (int i = 0; i < server_config().enemies.vampire.pairs; ++i) {
        spawn_pair(level);
    }
}

void VampireSpawner::spawn_pair(DungeonLevel& level) const {
    std::optional<Position> first = level.spawn_cell();
    if (!first) {
        return;
    }
    std::optional<Position> second = level.spawn_cell([&first](const Position& pos) {
        return std::abs(pos.x - first->x) + std::abs(pos.y - first->y) == 1;
    });
    if (!second) {
        return;
    }
    auto vampire =
            std::make_unique<Vampire>(level.new_enemy_id(), *first, level, level.get_visitors(),
                                      VampireKind::Vampire, level.new_seed());
    auto vampiress =
            std::make_unique<Vampire>(level.new_enemy_id(), *second, level, level.get_visitors(),
                                      VampireKind::Vampiress, level.new_seed());
    vampire->pair_with(*vampiress);
    level.spawn(std::move(vampire));
    level.spawn(std::move(vampiress));
}
