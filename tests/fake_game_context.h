#ifndef FAKE_GAME_CONTEXT_H
#define FAKE_GAME_CONTEXT_H

#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"
#include "tests/world_helpers.h"

// GameContext sin Game: tiene un jugador ya conectado y anota lo que le piden los comandos.
class FakeGameContext: public GameContext, public PlayerSessions, public PlayerMovement {
public:
    static constexpr uint32_t PLAYER_ID = 7;

    struct Added {
        uint32_t playerId;
        std::string characterName;
        std::shared_ptr<Outbox> outbox;
    };

    PlayerSessions& sessions() override { return *this; }
    PlayerMovement& movement() override { return *this; }

    void move_player(uint32_t player_id, Direction dir) override {
        if (player_id == PLAYER_ID) {
            player.set_direction(dir);
        }
    }

    void enter_portal(uint32_t player_id) override { entered_portal.push_back(player_id); }

    void add_player(uint32_t player_id, const GameMember& member,
                    std::shared_ptr<Outbox> outbox) override {
        added.push_back(Added{player_id, member.characterName, std::move(outbox)});
    }

    void remove_player(uint32_t player_id) override { removed.push_back(player_id); }

    TestLocation location;
    Player player{PLAYER_ID, Position{0, 0}, location, "jero", 10, 1.0f, 0, 0};
    std::vector<Added> added;
    std::vector<uint32_t> removed;
    std::vector<uint32_t> entered_portal;
};

#endif  // FAKE_GAME_CONTEXT_H
