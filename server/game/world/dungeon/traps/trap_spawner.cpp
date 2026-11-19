#include "server/game/world/dungeon/traps/trap_spawner.h"

#include <memory>
#include <optional>
#include <random>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/dungeon_level.h"
#include "server/game/world/dungeon/traps/flamethrower.h"
#include "server/game/world/dungeon/traps/spikes.h"

namespace {
// Si en `cell` hay pared: terreno que no se camina, o el borde del nivel. Una antorcha o un barril
// tapan la celda pero no son pared: el lanzallamas no se cuelga de ellos.
bool is_wall(const Location& level, const Position& cell) {
    const Grid& grid = level.get_grid();
    return !grid.in_bounds(cell) || !is_walkable_terrain(grid.at(cell).get_type());
}

// Hacia dónde tira el chorro el lanzallamas en `cell`: para el lado contrario a su pared. NONE si
// no tiene pared arriba, a la izquierda ni a la derecha.
Direction facing_from_wall(const Location& level, const Position& cell) {
    if (is_wall(level, step(cell, Direction::UP))) {
        return Direction::DOWN;
    }
    if (is_wall(level, step(cell, Direction::LEFT))) {
        return Direction::RIGHT;
    }
    if (is_wall(level, step(cell, Direction::RIGHT))) {
        return Direction::LEFT;
    }
    return Direction::NONE;
}

// Un momento del ciclo al azar, para que no se activen todas juntas
float random_start(DungeonLevel& level, const ServerConfig::Trap& stats) {
    std::mt19937 rng(level.new_seed());
    return std::uniform_real_distribution<float>(0, stats.active_time + stats.idle_time)(rng);
}
}  // namespace

void TrapSpawner::spawn(DungeonLevel& level) const {
    const ServerConfig::Traps& config = server_config().traps;
    for (int i = 0; i < config.spikes_count; ++i) {
        spawn_spikes(level, random_start(level, config.spikes));
    }
    for (int i = 0; i < config.flamethrowers_count; ++i) {
        spawn_flamethrower(level, random_start(level, config.flamethrower.trap));
    }
}

void TrapSpawner::spawn_spikes(DungeonLevel& level, float start) const {
    std::optional<Position> cell = level.spawn_cell();
    if (cell) {
        level.spawn(std::make_unique<Spikes>(level.new_enemy_id(), *cell, level,
                                             level.get_visitors(), start));
    }
}

void TrapSpawner::spawn_flamethrower(DungeonLevel& level, float start) const {
    std::optional<Position> cell = level.spawn_cell([&level](const Position& pos) {
        return facing_from_wall(level, pos) != Direction::NONE;
    });
    if (cell) {
        level.spawn(std::make_unique<Flamethrower>(level.new_enemy_id(), *cell, level,
                                                   level.get_visitors(),
                                                   facing_from_wall(level, *cell), start));
    }
}
