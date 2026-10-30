#include "server/game/game.h"

#include <chrono>
#include <stdexcept>
#include <thread>
#include <utility>
#include <variant>

Queue<Action>& Game::actions() { return inbox; }

uint32_t Game::add_player(Queue<ServerMessage>& outbox) {
    std::lock_guard<std::mutex> lock(mutex);
    uint32_t player_id = next_player_id++;
    PlayerState state{player_id, 0, 0, INITIAL_LIFE, INITIAL_ENERGY, 0};
    players.emplace(player_id, PlayerSlot{state, &outbox});
    return player_id;
}

void Game::remove_player(uint32_t player_id) {
    std::lock_guard<std::mutex> lock(mutex);
    players.erase(player_id);
}

void Game::run() {
    while (should_keep_running()) {
        process_actions();
        broadcast_state();
        std::this_thread::sleep_for(std::chrono::milliseconds(TICK_MS));
    }
}

void Game::stop() {
    Thread::stop();
    try {
        inbox.close();
    } catch (const std::runtime_error&) {}
}

void Game::process_actions() {
    Action action{};
    try {
        while (inbox.try_pop(action)) {
            std::lock_guard<std::mutex> lock(mutex);
            std::visit([this, &action](const auto& command) { apply(action.player_id, command); },
                       action.message);
        }
    } catch (const ClosedQueue&) {}
}

// Validación de ejemplo: el servidor decide la nueva posición, el cliente solo pide.
void Game::apply(uint32_t player_id, const MoveCommand& move) {
    std::map<uint32_t, PlayerSlot>::iterator slot = players.find(player_id);
    if (slot == players.end()) {
        return;
    }
    PlayerState& state = slot->second.state;
    switch (move.direction) {
        case Direction::Up:
            if (state.y > 0) {
                --state.y;
            }
            break;
        case Direction::Down:
            ++state.y;
            break;
        case Direction::Left:
            if (state.x > 0) {
                --state.x;
            }
            break;
        case Direction::Right:
            ++state.x;
            break;
    }
}

void Game::broadcast_state() {
    std::lock_guard<std::mutex> lock(mutex);
    for (std::pair<const uint32_t, PlayerSlot>& receiver: players) {
        for (const std::pair<const uint32_t, PlayerSlot>& other: players) {
            try {
                receiver.second.outbox->push(ServerMessage{other.second.state});
            } catch (const ClosedQueue&) {}
        }
    }
}