#ifndef GAME_REGISTRY_H
#define GAME_REGISTRY_H

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>


#include <memory>
#include "server/game/game.h"
#include "common/lobby_dto.h"
#include "server/persistence/game_store.h"

// Monitor con las partidas conocidas por el servidor y el índice userId -> partidas.
// El índice se arma desde el GameStore, no desde lo que está corriendo: un usuario ve sus
// partidas aunque nadie más esté conectado.
class GameRegistry {
public:
    GameRegistry(std::vector<WorldInfo> worlds, GameStore& store, uint8_t max_players);

    GameRegistry(const GameRegistry&) = delete;
    GameRegistry& operator=(const GameRegistry&) = delete;
    GameRegistry(GameRegistry&&) = delete;
    GameRegistry& operator=(GameRegistry&&) = delete;

    const std::vector<WorldInfo>& list_worlds() const;

    std::vector<SavedGameInfo> games_of(const UserId& user_id) const;

    // std::nullopt si el mundo no está entre los cargados.
    std::optional<uint32_t> create_game(const std::string& game_name, const std::string& world_id,
                                        bool is_multiplayer, const GameMember& owner);

    // Devuelve la partida en ejecución, creándola y arrancando su hilo si todavía no corre.
    Game& running_game(uint32_t game_id);

private:
    const std::vector<WorldInfo> worlds;
    const uint8_t max_players;
    GameStore& store;

    std::map<uint32_t, GameRecord> games;
    std::map<UserId, std::set<uint32_t>> games_by_user;
    uint32_t next_game_id = 1;
    mutable std::mutex mutex;

    bool world_exists(const std::string& world_id) const;
    void add(const GameRecord& game);

    std::map<uint32_t, std::unique_ptr<Game>> running;
};

#endif  // GAME_REGISTRY_H
