#include "server/game/world/world.h"

#include <algorithm>
#include <optional>
#include <utility>

#include "server/game/characters/player.h"

World::World(std::unique_ptr<Farm> farm, std::unique_ptr<Town> town, Dungeon dungeon,
             const Position& spawn):
        farm(std::move(farm)), town(std::move(town)), dungeon(std::move(dungeon)), spawn(spawn) {}

Farm& World::get_farm() { return *farm; }

Town& World::get_town() { return *town; }

Dungeon& World::get_dungeon() { return dungeon; }

const Position& World::get_spawn() const { return spawn; }

void World::add_player(Player& player) {
    player.get_location().add_entity(player);
    players.push_back(&player);
}

void World::remove_player(const Player& player) {
    player.get_location().remove_entity(player);
    players.erase(std::remove(players.begin(), players.end(), &player), players.end());
}

Location& World::location_of(const Player& player) const { return player.get_location(); }

void World::travel(Player& player, const Portal& portal) {
    player.relocate(*portal.target, portal.arrival);
}

void World::update(float dt) {
    farm->update(dt);
    town->update(dt);
    dungeon.update(dt);
    check_portals();
}

void World::check_portals() {
    for (Player* player: players) {
        if (!player->has_stepped()) {
            continue;
        }
        std::optional<Portal> portal = location_of(*player).portal_at(player->get_position());
        if (portal) {
            travel(*player, *portal);
        }
    }
}
