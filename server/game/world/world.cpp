#include "server/game/world/world.h"

#include <algorithm>
#include <cstdlib>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/config/server_config.h"

namespace {
// Si `cell` está pegada a la del jugador (también en diagonal), donde sea que esté.
bool next_to(const Player& player, const Position& cell) {
    const Position& here = player.get_position();
    return std::abs(cell.x - here.x) <= 1 && std::abs(cell.y - here.y) <= 1;
}
}  // namespace

World::World(std::unique_ptr<Farm> farm, std::unique_ptr<Town> town, Dungeon dungeon):
        farm(std::move(farm)),
        town(std::move(town)),
        dungeon(std::move(dungeon)),
        movement(*this->farm, players) {}

Farm& World::get_farm() { return *farm; }

Town& World::get_town() { return *town; }

Dungeon& World::get_dungeon() { return dungeon; }

const Dungeon& World::get_dungeon() const { return dungeon; }

void World::spawn_player(uint32_t player_id, std::string name) {
    SpawnPoint spawn = movement.spawn_point(player_id);
    auto back = away.find(player_id);
    if (back != away.end()) {
        std::unique_ptr<Player> player = std::move(back->second);
        away.erase(back);
        // Fuera del mapa no estaba en ninguna Location: lie_in lo da de alta en la cabaña
        player->lie_in(spawn);
        players[player_id] = std::move(player);
    } else {
        const ServerConfig::Player& stats = server_config().player;
        auto player = std::make_unique<Player>(player_id, cell_of(spawn.point), spawn.location,
                                               std::move(name), stats.health, stats.speed,
                                               stats.gold, stats.energy);
        player->place_at(spawn.point);
        auto saved = stored.find(player_id);
        if (saved != stored.end()) {
            player->restore(saved->second);
        }
        add_player(std::move(player));
    }
    // Su cama puede haber aparecido donde estaba parado otro
    movement.free_trapped();
}

SavedGame World::saved() const {
    SavedGame saved{clock.get_day(), farm->saved(), stored};
    for (const auto* group: {&players, &away}) {
        for (const auto& [player_id, player]: *group) {
            saved.players[player_id] = player->saved();
        }
    }
    return saved;
}

void World::restore(const SavedGame& saved) {
    clock.start_day(saved.day);
    farm->restore(saved.farm);
    stored = saved.players;
}

Player& World::add_player(std::unique_ptr<Player> player) {
    uint32_t player_id = player->get_id();
    away.erase(player_id);
    auto old = players.find(player_id);
    if (old != players.end()) {
        old->second->get_location().remove_entity(*old->second);
        players.erase(old);
    }
    player->get_location().add_entity(*player);
    return *players.emplace(player_id, std::move(player)).first->second;
}

void World::remove_player(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        player->second->get_location().remove_entity(*player->second);
        player->second->set_direction(Direction::NONE);
        away[player_id] = std::move(player->second);
        players.erase(player);
    }
}

Player* World::find_player(uint32_t player_id) {
    auto player = players.find(player_id);
    return player == players.end() ? nullptr : player->second.get();
}

bool World::reaches(const Player& player, const Position& cell) const {
    return &player.get_location() == farm.get() && next_to(player, cell);
}

bool World::dig(const Player& player, const Position& cell) {
    return reaches(player, cell) && farm->dig(cell);
}

bool World::plant(const Player& player, const Position& cell, CropType crop) {
    return reaches(player, cell) && farm->plant(cell, crop);
}

bool World::water(const Player& player, const Position& cell) {
    return reaches(player, cell) && farm->water(cell);
}

const Outcome& World::harvest(Player& player) {
    if (&player.get_location() != farm.get()) {
        return Outcome::not_here();
    }
    return farm->harvest(player.facing_cell(), player);
}

const Outcome& World::milk(Player& player) { return milk_nearest_cow(farm->get_animals(), player); }

bool World::attack(Player& player, const Position& cell) {
    DungeonLevel* level = dungeon.level_at(player.get_location());
    if (level == nullptr || !next_to(player, cell)) {
        return false;
    }
    Enemy* enemy = level->enemy_near(cell);
    return enemy != nullptr && player.strike(*enemy);
}

Movement& World::get_movement() { return movement; }

const Location& World::location_of(uint32_t player_id) const {
    return players.at(player_id)->get_location();
}

const std::map<uint32_t, std::unique_ptr<Player>>& World::get_players() const { return players; }

const std::vector<std::unique_ptr<Animal>>& World::get_animals() const {
    return farm->get_animals();
}

const Cabin& World::get_cabin() const { return farm->get_cabin(); }

const GameClock& World::get_clock() const { return clock; }

void World::start_new_day() {
    farm->start_new_day();
    for (const auto& [player_id, player]: players) {
        bool slept = !player->is_unconscious() && farm->get_cabin().is_in_bed(*player);
        player->wake_up(farm->bed_spawn(player_id), slept);
    }
    for (const auto& [player_id, player]: away) {
        player->rest(false);
    }
}

bool World::everyone_unconscious() const {
    return !players.empty() && std::all_of(players.begin(), players.end(), [](const auto& entry) {
        return entry.second->is_unconscious();
    });
}

bool World::update(float dt) {
    farm->update(dt);
    town->update(dt);
    std::vector<Creature*> characters;
    for (const auto& [player_id, player]: players) {
        characters.push_back(player.get());
    }
    dungeon.set_visitors(characters);
    dungeon.update(dt);
    movement.travel_through_portals();
    bool day_over = clock.update(dt);
    if (!day_over && everyone_unconscious()) {
        clock.end_day();
        day_over = true;
    }
    if (day_over) {
        start_new_day();
    }
    farm->update_time(clock.get_minute_of_day());
    return day_over;
}
