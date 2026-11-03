#include "server/game/world/dungeon/dungeon_level.h"

#include <algorithm>
#include <utility>

DungeonLevel::DungeonLevel(std::string name, Grid grid, int depth):
        Location(std::move(name), std::move(grid)), depth(depth) {}

int DungeonLevel::get_depth() const { return depth; }

void DungeonLevel::spawn(std::unique_ptr<Enemy> enemy) {
    add_entity(*enemy);
    enemies.push_back(std::move(enemy));
}

void DungeonLevel::update(float dt) {
    update_entities(dt);
    remove_dead_enemies();
}

void DungeonLevel::remove_dead_enemies() {
    auto dead = std::stable_partition(
            enemies.begin(), enemies.end(),
            [](const std::unique_ptr<Enemy>& enemy) { return enemy->is_alive(); });
    for (auto it = dead; it != enemies.end(); ++it) {
        remove_entity(**it);
    }
    enemies.erase(dead, enemies.end());
}
