#include "server/game/game.h"

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>
#include <variant>
#include <vector>

#include "common/logger.h"
#include "server/consts.h"
#include "server/game/world/world_builder.h"

namespace {
constexpr std::chrono::nanoseconds TICK_DURATION =
        std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::seconds(1)) /
        ServerConstants::TICK_RATE;
const float TICK_SECONDS = std::chrono::duration<float>(TICK_DURATION).count();

// Un solo eje por vez: con una diagonal camina en horizontal.
Direction direction_of(const Request& request) {
    if (request.dir_x != 0) {
        return request.dir_x > 0 ? Direction::RIGHT : Direction::LEFT;
    }
    if (request.dir_y != 0) {
        return request.dir_y > 0 ? Direction::DOWN : Direction::UP;
    }
    return Direction::NONE;
}

PlayerState state_of(uint32_t player_id, const Player& player) {
    Position pos = player.get_position();
    Position next = step(pos, player.get_direction());
    return PlayerState{player_id,
                       player.get_location().get_name(),
                       static_cast<uint16_t>(pos.x),
                       static_cast<uint16_t>(pos.y),
                       static_cast<int8_t>(next.x - pos.x),
                       static_cast<int8_t>(next.y - pos.y),
                       static_cast<uint16_t>(player.get_progress() * PROGRESS_SCALE),
                       static_cast<uint16_t>(player.get_health()),
                       static_cast<uint16_t>(player.get_energy()),
                       static_cast<uint32_t>(player.get_gold())};
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
    std::chrono::steady_clock::time_point next = std::chrono::steady_clock::now();
    try {
        while (should_keep_running()) {
            process_events();
            world->update(TICK_SECONDS);
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
    remove(joined.playerId);
    auto character = std::make_unique<Player>(
            joined.playerId, world->get_spawn(), world->get_farm(), joined.member.characterName,
            ServerConstants::INITIAL_HEALTH, ServerConstants::PLAYER_SPEED,
            ServerConstants::INITIAL_GOLD, ServerConstants::INITIAL_ENERGY);
    world->add_player(*character);
    players.emplace(joined.playerId, ConnectedPlayer{joined.outbox, std::move(character)});
    send(*joined.outbox, LocationData{world_id});
}

void Game::handle(const PlayerAction& action) {
    auto player = players.find(action.playerId);
    if (player == players.end() || action.request.tipo != Operation::Move) {
        return;
    }
    player->second.character->set_direction(direction_of(action.request));
}

void Game::handle(const PlayerLeft& left) {
    auto player = players.find(left.playerId);
    if (player == players.end()) {
        return;
    }
    try {
        player->second.outbox->close();
    } catch (const std::runtime_error&) {}
    remove(left.playerId);
}

void Game::remove(uint32_t player_id) {
    auto player = players.find(player_id);
    if (player != players.end()) {
        world->remove_player(*player->second.character);
        players.erase(player);
    }
}

void Game::broadcast_snapshot() {
    Snapshot snapshot{tick, {}};
    for (const auto& [player_id, player]: players) {
        snapshot.players.push_back(state_of(player_id, *player.character));
    }
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
