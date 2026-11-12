#include "server/game/game.h"

#include <chrono>
#include <cmath>
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

uint32_t to_wire(float cells) { return static_cast<uint32_t>(std::lround(cells * POSITION_SCALE)); }

PlayerState state_of(uint32_t player_id, const Player& player) {
    Position delta = step(Position{0, 0}, player.get_direction());
    return PlayerState{player_id,
                       player.get_location().get_name(),
                       to_wire(player.get_x()),
                       to_wire(player.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y),
                       static_cast<uint16_t>(player.get_health()),
                       static_cast<uint16_t>(player.get_energy()),
                       static_cast<uint32_t>(player.get_gold())};
}

EntityState entity_of(uint32_t player_id, const Player& player) {
    Position delta = step(Position{0, 0}, player.get_direction());
    return EntityState{player_id,
                       EntityType::Player,
                       player.get_location().get_name(),
                       to_wire(player.get_x()),
                       to_wire(player.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y)};
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
    std::shared_ptr<GameCommand> command;
    while (inbox.try_pop(command)) {
        try {
            command->execute(*this);
        } catch (const std::exception& e) {
            Logger::error("Server error: {}", e.what());
        }
    }
}

Player* Game::find_player(uint32_t player_id) {
    auto player = players.find(player_id);
    return player == players.end() ? nullptr : player->second.character.get();
}

void Game::add_player(uint32_t player_id, const GameMember& member,
                      std::shared_ptr<Outbox> outbox) {
    remove(player_id);
    auto character = std::make_unique<Player>(
            player_id, world->get_spawn(), world->get_farm(), member.characterName,
            ServerConstants::INITIAL_HEALTH, ServerConstants::PLAYER_SPEED,
            ServerConstants::INITIAL_GOLD, ServerConstants::INITIAL_ENERGY);
    world->add_player(*character);
    send(*outbox, LocationData{world_id});
    PlayerInfo info{player_id, member.characterName, member.appearance};
    for (const auto& [other_id, other]: players) {
        send(*outbox, other.info);
        send(*other.outbox, info);
    }
    send(*outbox, info);
    players.emplace(player_id,
                    ConnectedPlayer{std::move(outbox), std::move(character), std::move(info)});
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
        world->remove_player(*player->second.character);
        players.erase(player);
    }
}

void Game::broadcast_snapshot() {
    Snapshot snapshot{tick, {}, {}};
    for (const auto& [player_id, player]: players) {
        snapshot.players.push_back(state_of(player_id, *player.character));
        snapshot.entities.push_back(entity_of(player_id, *player.character));
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
