#ifndef FAKE_GAME_CONTEXT_H
#define FAKE_GAME_CONTEXT_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"
#include "tests/world_helpers.h"

// GameContext sin Game: tiene un jugador ya conectado y anota lo que le piden los comandos.
class FakeGameContext:
        public GameContext,
        public PlayerSessions,
        public PlayerMovement,
        public PlayerCharacters,
        public PlayerFarming {
public:
    static constexpr uint32_t PLAYER_ID = 7;

    struct Added {
        uint32_t playerId;
        std::string characterName;
        std::shared_ptr<Outbox> outbox;
    };

    PlayerSessions& sessions() override { return *this; }
    PlayerMovement& movement() override { return *this; }
    PlayerCharacters& characters() override { return *this; }
    PlayerFarming& farming() override { return *this; }

    Player* find_player(uint32_t player_id) override {
        return player_id == PLAYER_ID ? &player : nullptr;
    }

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

    void send_to(uint32_t player_id, const GameMessage& message) override {
        sent.emplace_back(player_id, message);
    }

    void broadcast(const GameMessage& message) override { broadcasted.push_back(message); }

    std::optional<uint32_t> find_player_by_name(const std::string& character_name) override {
        auto found = connected.find(character_name);
        if (found == connected.end()) {
            return std::nullopt;
        }
        return found->second;
    }

    bool dig(const Player&) override { return false; }

    bool plant(const Player&, CropType) override { return false; }

    bool water(const Player&) override { return false; }

    const Outcome& harvest(Player&) override { return Outcome::not_here(); }

    const Outcome& milk(Player& player) override { return milk_nearest_cow(animals, player); }

    // Los animales que ve el comando: los tests los agregan en la misma Location que el jugador.
    std::vector<std::unique_ptr<Animal>> animals;

    // Lo que los comandos le mandaron a todos, en orden.
    std::vector<GameMessage> broadcasted;
    // Nombre de personaje -> id de los jugadores conectados. El propio se llama "jero".
    std::map<std::string, uint32_t> connected{{"jero", PLAYER_ID}};

    // Lo que los comandos le mandaron a cada jugador, en orden.
    std::vector<std::pair<uint32_t, GameMessage>> sent;

    TestLocation location;
    Player player{PLAYER_ID, Position{0, 0}, location, "jero", 10, 1.0f, 0, 0};
    std::vector<Added> added;
    std::vector<uint32_t> removed;
    std::vector<uint32_t> entered_portal;
};

#endif  // FAKE_GAME_CONTEXT_H
