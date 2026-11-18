#include "server/game/world/dungeon/dungeon_level.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/skeleton.h"
#include "server/game/world/point.h"

namespace {
// Los ids de los enemigos de cada nivel arrancan en (depth + 1) * ENEMY_IDS: no se repiten entre
// niveles.
constexpr uint32_t ENEMY_IDS = 1000;
// Hasta dónde, del centro de la celda clickeada, cuenta un enemigo como "en esa celda"
constexpr float ENEMY_NEAR_CELL = 1.0f;
}  // namespace

DungeonLevel::DungeonLevel(std::string name, Grid grid, int depth, uint32_t seed):
        Location(std::move(name), std::move(grid)),
        depth(depth),
        rng(seed),
        next_enemy_id((static_cast<uint32_t>(depth) + 1) * ENEMY_IDS) {}

int DungeonLevel::get_depth() const { return depth; }

void DungeonLevel::spawn(std::unique_ptr<Enemy> enemy) {
    add_entity(*enemy);
    enemies.push_back(std::move(enemy));
}

Enemy* DungeonLevel::enemy_near(const Position& cell) const {
    Point center = center_of(cell);
    Enemy* nearest = nullptr;
    float nearest_distance = ENEMY_NEAR_CELL;
    for (const std::unique_ptr<Enemy>& enemy: enemies) {
        float distance = std::hypot(enemy->get_x() - center.x, enemy->get_y() - center.y);
        if (enemy->is_alive() && distance <= nearest_distance) {
            nearest = enemy.get();
            nearest_distance = distance;
        }
    }
    return nearest;
}

void DungeonLevel::set_visitors(std::vector<Creature*> players) {
    bool was_empty = visitors.empty();
    visitors = std::move(players);
    if (was_empty && !visitors.empty()) {
        repopulate();
    }
}

void DungeonLevel::update(float dt) {
    update_entities(dt);
    remove_gone_enemies();
}

void DungeonLevel::repopulate() {
    remove_enemies();
    for (int i = 0; i < server_config().enemies.skeleton.count; ++i) {
        std::optional<Position> cell = random_free_cell(
                rng, [this](const Position& pos) { return far_from_portals(pos); });
        if (!cell) {
            return;
        }
        spawn(std::make_unique<Skeleton>(next_enemy_id++, *cell, *this, visitors));
    }
}

void DungeonLevel::remove_enemies() {
    for (const std::unique_ptr<Enemy>& enemy: enemies) {
        remove_entity(*enemy);
    }
    enemies.clear();
}

bool DungeonLevel::far_from_portals(const Position& pos) const {
    float min_distance = server_config().enemies.spawn_distance;
    return std::all_of(portals.begin(), portals.end(), [&pos, min_distance](const Portal& portal) {
        return std::hypot(static_cast<float>(pos.x - portal.origin.x),
                          static_cast<float>(pos.y - portal.origin.y)) >= min_distance;
    });
}

void DungeonLevel::remove_gone_enemies() {
    // stable_partition reordena: adelante los que siguen, atrás los que se van (sin cambiar el
    // orden dentro de cada grupo). Devuelve dónde empiezan los que se van.
    auto gone = std::stable_partition(
            enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) { return !enemy->is_gone(); });
    // Los que se van dejan de ser entidades de la Location y se destruyen al sacarlos del vector
    for (auto it = gone; it != enemies.end(); ++it) {
        remove_entity(**it);
    }
    enemies.erase(gone, enemies.end());
}
