#include "server/game/world/world.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/config/server_config.h"

World::World(std::unique_ptr<Farm> farm, std::unique_ptr<Town> town, Dungeon dungeon):
        farm(std::move(farm)),
        town(std::move(town)),
        dungeon(std::move(dungeon)),
        movement(*this->farm, players) {}

Farm& World::get_farm() { return *farm; }

Town& World::get_town() { return *town; }

Dungeon& World::get_dungeon() { return dungeon; }

void World::spawn_player(uint32_t player_id, std::string name) {
    SpawnPoint spawn = movement.spawn_point(player_id);
    const ServerConfig::Player& stats = server_config().player;
    auto player = std::make_unique<Player>(player_id, cell_of(spawn.point), spawn.location,
                                           std::move(name), stats.health, stats.speed, stats.gold,
                                           stats.energy);
    player->place_at(spawn.point);
    add_player(std::move(player));
    // Su cama puede haber aparecido donde estaba parado otro
    movement.free_trapped();
}

Player& World::add_player(std::unique_ptr<Player> player) {
    uint32_t player_id = player->get_id();
    remove_player(player_id);
    player->get_location().add_entity(*player);
    return *players.emplace(player_id, std::move(player)).first->second;
}

void World::remove_player(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        player->second->get_location().remove_entity(*player->second);
        players.erase(player);
    }
}

Player* World::find_player(uint32_t player_id) {
    auto player = players.find(player_id);
    return player == players.end() ? nullptr : player->second.get();
}

const std::vector<std::unique_ptr<Animal>>& World::farm_animals() { return farm->get_animals(); }

Farm* World::find_farm() { return farm.get(); }

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
}

void World::update(float dt) {
    farm->update(dt);
    town->update(dt);
    std::vector<Creature*> characters;
    for (const auto& [player_id, player]: players) {
        characters.push_back(player.get());
    }
    dungeon.set_visitors(characters);
    dungeon.update(dt);
    movement.travel_through_portals();
    if (clock.update(dt)) {
        start_new_day();
    }
}
