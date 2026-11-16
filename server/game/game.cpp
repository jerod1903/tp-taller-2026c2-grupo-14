#include "server/game/game.h"

#include <chrono>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

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

Game::Game(std::string world_id, const stardew::maps::MapDefinition& map):
        world_id(std::move(world_id)), world(build_world(map)) {}

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
            world->update(tick_seconds);
            ++tick;
            broadcast_snapshot();
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
        try {
            command->execute(*this);
        } catch (const std::exception& e) {
            Logger::error("Server error: {}", e.what());
        }
    }
}

PlayerSessions& Game::sessions() { return *this; }

PlayerMovement& Game::movement() { return world->get_movement(); }

void Game::add_player(uint32_t player_id, const GameMember& member,
                      std::shared_ptr<Outbox> outbox) {
    remove(player_id);
    world->spawn_player(player_id, member.characterName);
    send(*outbox, LocationData{world_id, static_cast<uint16_t>(server_config().game.tick_rate),
                               server_config().game.max_players});
    PlayerInfo info{player_id, member.characterName, member.appearance};
    for (const auto& [other_id, other]: players) {
        send(*outbox, other.info);
        send(*other.outbox, info);
    }
    send(*outbox, info);
    players.emplace(player_id, ConnectedPlayer{std::move(outbox), std::move(info)});
}

void Game::remove_player(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player == players.end()) {
        return;
    }
    try {
        player->second.outbox->close();
    } catch (const std::runtime_error&) {}
    remove(player_id);
}

void Game::remove(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        world->remove_player(player_id);
        players.erase(player);
    }
}

void Game::broadcast_snapshot() {
    Snapshot snapshot = SnapshotBuilder::build(tick, *world);
    for (const auto& [player_id, player]: players) {
        send(*player.outbox, snapshot);
    }
}

void Game::send(Outbox& outbox, const GameMessage& message) {
    try {
        if (!outbox.try_push(message)) {
            Logger::debug("outbox llena: se descarta un mensaje");
        }
    } catch (const ClosedQueue&) {}
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
