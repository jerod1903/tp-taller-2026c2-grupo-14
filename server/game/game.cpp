#include "server/game/game.h"

#include <chrono>
#include <memory>
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

Game::Game(std::string world_id, const stardew::maps::MapDefinition& map):
        world(build_world(map)), player_sessions(std::move(world_id), *world) {}

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

PlayerSessions& Game::sessions() { return player_sessions; }

PlayerMovement& Game::movement() { return world->get_movement(); }

void Game::broadcast_snapshot() { player_sessions.broadcast(SnapshotBuilder::build(tick, *world)); }

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
