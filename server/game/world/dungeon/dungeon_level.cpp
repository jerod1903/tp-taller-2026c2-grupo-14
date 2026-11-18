#include "server/game/world/dungeon/dungeon_level.h"

#include <algorithm>
#include <cmath>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/enemies/skeleton.h"

namespace {
// Los ids de los enemigos de cada nivel arrancan en (depth + 1) * ENEMY_IDS: no se repiten entre
// niveles.
constexpr uint32_t ENEMY_IDS = 1000;
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
    ores.clear();
    for (int i = 0; i < server_config().enemies.skeleton.count; ++i) {
        std::optional<Position> cell = random_free_cell(
                rng, [this](const Position& pos) { return far_from_portals(pos); });
        if (!cell) {
            break;
        }
        spawn(std::make_unique<Skeleton>(next_enemy_id++, *cell, *this, visitors));
    }
    populate_ores();
}

bool DungeonLevel::is_walkable(const Position& pos) const {
    return Location::is_walkable(pos) && ores.find(pos) == ores.end();
}

const std::map<Position, DungeonLevel::OreNode>& DungeonLevel::get_ores() const { return ores; }

DungeonLevel::OreNode* DungeonLevel::ore_at(const Position& pos) {
    auto ore = ores.find(pos);
    return ore == ores.end() ? nullptr : &ore->second;
}

void DungeonLevel::remove_ore(const Position& pos) { ores.erase(pos); }

void DungeonLevel::populate_ores() {
    const ServerConfig::Mining& mining = server_config().mining;
    for (ItemType ore: {ItemType::Coal, ItemType::Lapis, ItemType::Silver, ItemType::GoldOre}) {
        if (ore == ItemType::GoldOre && depth < mining.gold_min_depth) {
            continue;
        }
        const ServerConfig::OreSpec& spec = mining.of(ore);
        for (int i = 0; i < spec.count; ++i) {
            std::optional<Position> cell = random_free_cell(
                    rng, [this](const Position& pos) { return far_from_portals(pos); });
            if (!cell) {
                return;
            }
            ores.emplace(*cell, OreNode{ore, spec.hits});
        }
    }
}

DungeonLevel::StrikeResult DungeonLevel::strike_ore(const Position& pos) {
    return strike_ore(pos, std::uniform_real_distribution<float>(0.0f, 1.0f)(rng));
}

DungeonLevel::StrikeResult DungeonLevel::strike_ore(const Position& pos, float roll) {
    OreNode* node = ore_at(pos);
    if (node == nullptr) {
        return {Strike::NoOre};
    }
    if (--node->hits_left > 0) {
        return {Strike::Chipped};
    }
    ItemType ore = node->ore;
    remove_ore(pos);
    if (ore == ItemType::Lapis && roll < server_config().mining.lapis_break_chance) {
        return {Strike::Crumbled};
    }
    return {Strike::Mined, ore};
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
