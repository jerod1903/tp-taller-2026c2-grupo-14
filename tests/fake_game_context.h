#ifndef FAKE_GAME_CONTEXT_H
#define FAKE_GAME_CONTEXT_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/game/commands/game_context.h"
#include "tests/world_helpers.h"

// GameContext sin Game: tiene un jugador ya conectado y anota lo que le piden los comandos.
class FakeGameContext: public GameContext {
public:
    static constexpr uint32_t PLAYER_ID = 7;

    struct Added {
        uint32_t playerId;
        std::string characterName;
        std::shared_ptr<Outbox> outbox;
    };

    Player* find_player(uint32_t player_id) override {
        return player_id == PLAYER_ID ? &player : nullptr;
    }

    void add_player(uint32_t player_id, const GameMember& member,
                    std::shared_ptr<Outbox> outbox) override {
        added.push_back(Added{player_id, member.characterName, std::move(outbox)});
    }

    void remove_player(uint32_t player_id) override { removed.push_back(player_id); }

    void send_to(uint32_t player_id, const GameMessage& message) override {
        sent.emplace_back(player_id, message);
    }

    // Lo que los comandos le mandaron a cada jugador, en orden.
    std::vector<std::pair<uint32_t, GameMessage>> sent;

    TestLocation location;
    Player player{PLAYER_ID, Position{0, 0}, location, "jero", 10, 1.0f, 0, 0};
    std::vector<Added> added;
    std::vector<uint32_t> removed;
};

#endif  // FAKE_GAME_CONTEXT_H
