#include "server/game/world/dungeon/dungeon_level.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <functional>
#include <memory>
#include <optional>
#include <utility>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/spawners/enemy_spawner.h"
#include "server/game/world/dungeon/traps/trap_spawner.h"
#include "server/game/world/point.h"

namespace {
// Los ids de los enemigos (y los de las llamas) de cada nivel van de (depth + 1) * ENEMY_IDS a
// (depth + 2) * ENEMY_IDS: no se repiten entre niveles. Cuando se acaban vuelven a empezar; para
// entonces los primeros ya no están.
constexpr uint32_t ENEMY_IDS = 1000;
// Hasta dónde, del centro de la celda clickeada, cuenta un enemigo como "en esa celda"
constexpr float ENEMY_NEAR_CELL = 1.0f;

constexpr int MAX_DEPTH = 9;

struct OreWeightCurve {
    ItemType ore;
    double start_weight;
    double end_weight;
};

constexpr std::array<OreWeightCurve, 4> ORE_CURVES = {{
        {ItemType::Coal, 55.0, 10.0},
        {ItemType::Lapis, 30.0, 25.0},
        {ItemType::Silver, 15.0, 40.0},
        {ItemType::GoldOre, 10.0, 25.0},
}};

double weight_for(const OreWeightCurve& curve, int depth, int gold_min_depth) {
    int start_depth = curve.ore == ItemType::GoldOre ? gold_min_depth : 0;
    int clamped = std::clamp(depth, start_depth, MAX_DEPTH);
    double t = static_cast<double>(clamped - start_depth) / (MAX_DEPTH - start_depth);
    return curve.start_weight + t * (curve.end_weight - curve.start_weight);
}

ItemType roll_ore(std::mt19937& rng, int depth, int gold_min_depth) {
    std::vector<ItemType> candidates;
    std::vector<double> weights;
    for (const auto& curve: ORE_CURVES) {
        if (curve.ore == ItemType::GoldOre && depth < gold_min_depth) {
            continue;
        }
        candidates.push_back(curve.ore);
        weights.push_back(weight_for(curve, depth, gold_min_depth));
    }
    std::discrete_distribution<std::size_t> dist(weights.begin(), weights.end());
    return candidates[dist(rng)];
}
}  // namespace

DungeonLevel::DungeonLevel(std::string name, Grid grid, int depth, uint32_t seed):
        Location(std::move(name), std::move(grid)), depth(depth), rng(seed) {}

int DungeonLevel::get_depth() const { return depth; }

void DungeonLevel::spawn(std::unique_ptr<Enemy> enemy) {
    add_entity(*enemy);
    enemies.push_back(std::move(enemy));
}

void DungeonLevel::spawn(std::unique_ptr<Trap> trap) {
    add_entity(*trap);
    traps.push_back(std::move(trap));
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

const std::vector<std::unique_ptr<Enemy>>& DungeonLevel::get_enemies() const { return enemies; }

const std::vector<std::unique_ptr<Trap>>& DungeonLevel::get_traps() const { return traps; }

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
    remove_traps();
    if (!ores.empty()) {
        ores.clear();
        changed = true;
    }
    populate_ores();
    TrapSpawner().spawn(*this);
    random_spawner(rng).spawn(*this);
}


bool DungeonLevel::is_walkable(const Position& pos) const {
    return Location::is_walkable(pos) && ores.find(pos) == ores.end();
}

const std::map<Position, DungeonLevel::OreNode>& DungeonLevel::get_ores() const { return ores; }

DungeonLevel::OreNode* DungeonLevel::ore_at(const Position& pos) {
    auto ore = ores.find(pos);
    return ore == ores.end() ? nullptr : &ore->second;
}

void DungeonLevel::remove_ore(const Position& pos) {
    if (ores.erase(pos) > 0) {
        changed = true;
    }
}

void DungeonLevel::populate_ores() {
    const ServerConfig::Mining& mining = server_config().mining;
    for (int i = 0; i < mining.ores_per_level; ++i) {
        std::optional<Position> cell = random_free_cell(
                rng, [this](const Position& pos) { return far_from_portals(pos); });
        if (!cell) {
            return;
        }
        ItemType ore = roll_ore(rng, depth, mining.gold_min_depth);
        ores.emplace(*cell, OreNode{ore, mining.of(ore).hits});
        changed = true;
    }
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
    bool spawn_orcs = ore == ItemType::Silver && roll < server_config().mining.silver_orc_chance;
    return {Strike::Mined, ore, spawn_orcs};
}

DungeonLevel::StrikeResult DungeonLevel::strike_ore(const Position& pos) {
    return strike_ore(pos, std::uniform_real_distribution<float>(0.0f, 1.0f)(rng));
}

std::optional<Position> DungeonLevel::spawn_cell(const std::function<bool(const Position&)>& also) {
    return random_free_cell(rng, [this, &also](const Position& pos) {
        return far_from_portals(pos) && (!also || also(pos));
    });
}

uint32_t DungeonLevel::new_enemy_id() {
    return (static_cast<uint32_t>(depth) + 1) * ENEMY_IDS + enemies_made++ % ENEMY_IDS;
}

uint32_t DungeonLevel::new_seed() { return rng(); }

void DungeonLevel::add_flame(const Flame& flame) { flames.push_back(&flame); }

void DungeonLevel::remove_flame(const Flame& flame) {
    flames.erase(std::remove(flames.begin(), flames.end(), &flame), flames.end());
}

const std::vector<const Flame*>& DungeonLevel::get_flames() const { return flames; }

uint32_t DungeonLevel::new_flame_id() {
    return (static_cast<uint32_t>(depth) + 1) * ENEMY_IDS + flames_made++ % ENEMY_IDS;
}

const std::vector<Creature*>& DungeonLevel::get_visitors() const { return visitors; }

void DungeonLevel::remove_enemies() {
    for (const std::unique_ptr<Enemy>& enemy: enemies) {
        remove_entity(*enemy);
    }
    enemies.clear();
}

void DungeonLevel::remove_traps() {
    for (const std::unique_ptr<Trap>& trap: traps) {
        remove_entity(*trap);
    }
    traps.clear();
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