#include "server/game/game.h"

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "common/logger.h"
#include "server/consts.h"

namespace {
constexpr std::chrono::nanoseconds TICK_DURATION =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) /
        ServerConstants::TICK_RATE;
}  // namespace

Game::Game(std::string world_id): world_id(std::move(world_id)) {}

Inbox& Game::get_inbox() { return inbox; }

void Game::start() {
    Thread::start();
    started = true;
}

void Game::run() {
    std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();
    try {
        while (should_keep_running()) {
            process_events();
            ++tick;
            broadcast_snapshot();
            next += TICK_DURATION;
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
    GameEvent event;
    while (inbox.try_pop(event)) {
        std::visit([this](const auto& e) { handle(e); }, event);
    }
}

void Game::handle(const PlayerJoined& joined) {
    players.insert_or_assign(joined.playerId, joined.outbox);
    send(*joined.outbox, LocationData{world_id});
}

void Game::handle(const PlayerAction& action) {
    Logger::debug("jugador {}: acción {}", action.playerId, static_cast<int>(action.request.tipo));
}

void Game::handle(const PlayerLeft& left) {
    std::map<uint32_t, std::shared_ptr<Outbox>>::iterator player = players.find(left.playerId);
    if (player == players.end()) {
        return;
    }
    try {
        player->second->close();
    } catch (const std::runtime_error&) {}
    players.erase(player);
}

void Game::broadcast_snapshot() {
    Snapshot snapshot{tick, {}};
    for (const std::pair<const uint32_t, std::shared_ptr<Outbox>>& player: players) {
        snapshot.players.push_back(player.first);
    }
    for (const std::pair<const uint32_t, std::shared_ptr<Outbox>>& player: players) {
        send(*player.second, snapshot);
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
