#include "server/game/world/movement.h"

#include <optional>

#include "server/game/characters/player.h"
#include "server/game/world/farm/farm.h"

Movement::Movement(Farm& farm, const std::map<uint32_t, std::unique_ptr<Player>>& players):
        farm(farm), players(players) {}

SpawnPoint Movement::spawn_point(uint32_t player_id) { return farm.bed_spawn(player_id); }

void Movement::move_player(uint32_t player_id, Direction dir) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        player->second->set_direction(dir);
    }
}

void Movement::travel_through_portals() {
    for (const auto& [player_id, player]: players) {
        // Los portales se disparan al entrar a la celda: el que llega sobre uno no rebota
        if (!player->has_stepped()) {
            continue;
        }
        std::optional<Portal> portal = player->get_location().portal_at(player->get_position());
        if (portal) {
            travel(*player, *portal);
        }
    }
}

void Movement::travel(Player& player, const Portal& portal) {
    player.relocate(*portal.target, portal.arrival);
}
