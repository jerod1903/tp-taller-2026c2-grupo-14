#ifndef GAME_REGISTRY_H
#define GAME_REGISTRY_H

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "common/lobby_dto.h"
#include "server/persistence/game_store.h"

// Monitor con las partidas conocidas por el servidor y el índice username -> partidas.
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

    std::vector<SavedGameInfo> games_of(const std::string& username) const;

    // Toda partida arranca single, con el creador ya conectado. std::nullopt si el mundo no está
    // entre los cargados.
    std::optional<uint32_t> create_game(const std::string& game_name, const std::string& world_id,
                                        const GameMember& owner);

    CheckInviteCodeResponse check_invite_code(const std::string& code,
                                              const std::string& username) const;

    // Suma un miembro nuevo a la partida del código y lo deja conectado a ella.
    JoinGameResponse join_game(const std::string& code, const GameMember& member);

    // Conecta a un miembro existente. GameNotFound también si el usuario no participa.
    LobbyStatus load_game(uint32_t game_id, const std::string& username);

    // El usuario se desconectó de la partida: puede volver a entrar con load_game.
    void leave_game(uint32_t game_id, const std::string& username);

private:
    const std::vector<WorldInfo> worlds;
    // Cupo de una partida cuando pasa a multi.
    const uint8_t max_players;
    GameStore& store;

    std::map<uint32_t, GameRecord> games;
    std::map<std::string, std::set<uint32_t>> games_by_user;
    std::map<std::string, uint32_t> games_by_code;
    std::map<uint32_t, std::set<std::string>> connected;
    uint32_t next_game_id = 1;
    mutable std::mutex mutex;

    bool world_exists(const std::string& world_id) const;
    std::string world_name(const std::string& world_id) const;
    void add(const GameRecord& game);
    // Las que siguen requieren tener el lock tomado.
    static bool is_member(const GameRecord& game, const std::string& username);
    static bool name_taken(const GameRecord& game, const std::string& character_name);
};

#endif  // GAME_REGISTRY_H
