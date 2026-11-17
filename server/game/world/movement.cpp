#include "server/game/world/movement.h"

#include <optional>
#include <queue>
#include <vector>

#include "server/game/characters/player.h"
#include "server/game/world/farm/farm.h"
#include "server/game/world/location.h"

Movement::Movement(Farm& farm, const std::map<uint32_t, std::unique_ptr<Player>>& players):
        farm(farm), players(players) {}

SpawnPoint Movement::spawn_point(uint32_t player_id) { return farm.bed_spawn(player_id); }

void Movement::move_player(uint32_t player_id, Direction dir) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        player->second->set_direction(dir);
    }
}

void Movement::enter_portal(uint32_t player_id) {
    auto it = players.find(player_id);
    if (it == players.end()) {
        return;
    }
    Player& player = *it->second;
    const Position& here = player.get_position();
    for (const Position& cell: {here, step(here, Direction::UP), step(here, Direction::DOWN),
                                step(here, Direction::LEFT), step(here, Direction::RIGHT)}) {
        std::optional<Portal> portal = player.get_location().portal_at(cell);
        if (portal && portal->trigger == PortalTrigger::Action) {
            travel(player, *portal);
            player.set_direction(Direction::NONE);
            return;
        }
    }
}

void Movement::travel_through_portals() {
    for (const auto& [player_id, player]: players) {
        // Los portales se disparan al entrar a la celda: el que llega sobre uno no rebota
        if (!player->has_stepped()) {
            continue;
        }
        std::optional<Portal> portal = player->get_location().portal_at(player->get_position());
        if (portal && portal->trigger == PortalTrigger::Step) {
            travel(*player, *portal);
        }
    }
}

void Movement::free_trapped() {
    for (const auto& [player_id, player]: players) {
        if (!player->is_trapped()) {
            continue;
        }
        std::optional<Position> cell =
                nearest_free_cell(player->get_location(), player->get_position());
        if (cell) {
            player->relocate(player->get_location(), *cell);
        }
    }
}

std::optional<Position> Movement::nearest_free_cell(const Location& location,
                                                    const Position& from) {
    const Grid& grid = location.get_grid();
    std::vector<bool> seen(static_cast<std::size_t>(grid.get_width() * grid.get_height()), false);
    auto index = [&grid](const Position& pos) {
        return static_cast<std::size_t>(pos.y * grid.get_width() + pos.x);
    };
    std::queue<Position> pending;
    if (grid.in_bounds(from)) {
        pending.push(from);
        seen[index(from)] = true;
    }
    // Atraviesa paredes: la celda encerrada puede no tener vecinos libres
    while (!pending.empty()) {
        Position pos = pending.front();
        pending.pop();
        if (location.is_walkable(pos)) {
            return pos;
        }
        for (Direction dir: {Direction::UP, Direction::DOWN, Direction::LEFT, Direction::RIGHT}) {
            Position next = step(pos, dir);
            if (grid.in_bounds(next) && !seen[index(next)]) {
                seen[index(next)] = true;
                pending.push(next);
            }
        }
    }
    return std::nullopt;
}

void Movement::travel(Player& player, const Portal& portal) {
    player.relocate(*portal.target, cell_of(portal.arrival));
    player.place_at(portal.arrival);
}
