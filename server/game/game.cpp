#include "server/game/game.h"

#include <chrono>
#include <memory>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>

#include "common/logger.h"
#include "server/config/server_config.h"
#include "server/game/snapshot_builder.h"
#include "server/game/world/world_builder.h"

namespace {
std::chrono::nanoseconds tick_duration() {
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) /
           server_config().game.tick_rate;
}
}  // namespace

Game::Game(std::string world_id, const stardew::maps::MapDefinition& map, uint32_t game_id,
           GameStore& store, const std::optional<SavedGame>& saved):
        game_id(game_id),
        store(store),
        world(build_world(map)),
        market(build_market(map, *world)),
        player_sessions(std::move(world_id), *world) {
    if (saved) {
        world->restore(*saved);
    }
}

Inbox& Game::get_inbox() { return inbox; }

void Game::start() {
    Thread::start();
    started = true;
}

void Game::run() {
    const std::chrono::nanoseconds period = tick_duration();
    const float tick_seconds = std::chrono::duration<float>(period).count();
    std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();
    try {
        while (should_keep_running()) {
            process_events();
            if (vote) {
                update_vote();
            } else if (world->update(tick_seconds)) {
                open_vote();
            }
            ++tick;
            broadcast_snapshot();
            if (world->get_farm().take_changes()) {
                player_sessions.broadcast(SnapshotBuilder::farm_state(world->get_farm()));
            }
            Dungeon& dungeon = world->get_dungeon();
            for (int depth = 0; depth < Dungeon::LEVEL_COUNT; ++depth) {
                DungeonLevel& level = dungeon.level(depth);
                if (level.take_changes()) {
                    player_sessions.broadcast(SnapshotBuilder::dungeon_state(level));
                }
            }
            next += period;
            std::chrono::steady_clock::time_point now = std::chrono::steady_clock::now();
            if (next < now) {
                next = now;  // Atrasado: no intenta recuperar los ticks perdidos.
            } else {
                std::this_thread::sleep_until(next);
            }
        }
    } catch (const ClosedQueue&) {}
}

void Game::process_events() {
    std::shared_ptr<GameCommand> command;
    while (inbox.try_pop(command)) {
        if (vote && !command->runs_while_day_ended()) {
            continue;
        }
        try {
            command->execute(*this);
        } catch (const std::exception& e) {
            Logger::error("Server error: {}", e.what());
        }
    }
}

PlayerSessions& Game::sessions() { return player_sessions; }

PlayerMovement& Game::movement() { return world->get_movement(); }

PlayerCharacters& Game::characters() { return *world; }

PlayerCommerce& Game::commerce() { return market; }

PlayerFarming& Game::farming() { return *world; }

PlayerCombat& Game::combat() { return *world; }

PlayerDay& Game::day() { return *this; }

void Game::ready_for_next_day(uint32_t player_id) {
    if (vote && vote->vote(player_id)) {
        send_vote();
    }
}

void Game::end_day() { world->end_day(); }

void Game::open_vote() {
    vote.emplace(connected_players());
    send_vote();
}

void Game::update_vote() {
    // Le pregunta a la votación si cambió y si terminó, y decide acá: lo que sigue (avisar a los
    // jugadores, arrancar el día y guardar) es del Game, y DayVote no conoce al World ni al store.
    if (vote->sync(connected_players())) {
        send_vote();
    }
    if (vote->is_done()) {
        vote.reset();
        world->start_new_day();
        save();
    }
}

void Game::send_vote() {
    uint16_t day = world->get_clock().get_day();
    for (uint32_t player_id: connected_players()) {
        player_sessions.send_to(player_id, DayEnded{day, world->slept_in_bed(player_id),
                                                    vote->ready_count(), vote->total()});
    }
}

std::set<uint32_t> Game::connected_players() const {
    std::set<uint32_t> ids;
    for (const auto& [player_id, player]: world->get_players()) {
        ids.insert(player_id);
    }
    return ids;
}

void Game::broadcast_snapshot() { player_sessions.broadcast(SnapshotBuilder::build(tick, *world)); }

void Game::save() {
    try {
        store.save_game(game_id, world->saved());
        Logger::info("partida {} guardada (día {})", game_id, world->get_clock().get_day());
    } catch (const std::exception& e) {
        Logger::error("no se pudo guardar la partida {}: {}", game_id, e.what());
    }
}

void Game::stop() {
    Thread::stop();
    try {
        inbox.close();
    } catch (const std::runtime_error&) {}
}

Game::~Game() {
    stop();
    if (started) {
        join();
    }
}
