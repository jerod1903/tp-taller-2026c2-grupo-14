#ifndef GAME_REGISTRY_H
#define GAME_REGISTRY_H

#include <cstdint>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <random>
#include <set>
#include <string>
#include <vector>

#include "common/lobby_dto.h"
#include "server/game/game.h"
#include "server/game/world/world_loader.h"
#include "server/persistence/game_store.h"

// Monitor con las partidas conocidas por el servidor y el índice username -> partidas.
// El índice se arma desde el GameStore, no desde lo que está corriendo: un usuario ve sus
// partidas aunque nadie más esté conectado.
class GameRegistry {
public:
    GameRegistry(const std::vector<GameWorld>& worlds, GameStore& store, uint8_t max_players);

    GameRegistry(const GameRegistry&) = delete;
    GameRegistry& operator=(const GameRegistry&) = delete;
    GameRegistry(GameRegistry&&) = delete;
    GameRegistry& operator=(GameRegistry&&) = delete;

    const std::vector<WorldInfo>& list_worlds() const;

    std::vector<SavedGameInfo> games_of(const std::string& username) const;

    // El playerId de cada jugador es su posición en GameRecord::members, que solo crece: el
    // creador es siempre el primero.
    static constexpr uint32_t OWNER_PLAYER_ID = 0;

    // Toda partida arranca single, con el creador ya conectado. std::nullopt si el mundo no está
    // entre los cargados.
    std::optional<uint32_t> create_game(const std::string& game_name, const std::string& world_id,
                                        const GameMember& owner);

    // Ok, InvalidInviteCode, GameFull o AlreadyMember.
    LobbyStatus check_invite_code(const std::string& code, const std::string& username) const;

    // Suma un miembro nuevo a la partida del código y lo deja conectado a ella. Como load_game,
    // lanza el Game si no estaba corriendo (GameClosed si lo guardado no se puede leer).
    GameEnteredResponse join_game(const std::string& code, const GameMember& member);

    // Conecta a un miembro existente y lanza el Game de la partida si no estaba corriendo, con lo
    // que tenga guardado. GameNotFound también si el usuario no participa, WorldNotFound si el mapa
    // de la partida ya no se carga y GameClosed si lo guardado no se puede leer.
    GameEnteredResponse load_game(uint32_t game_id, const std::string& username);

    GameMember member_of(uint32_t game_id, uint32_t player_id) const;

    // Pasa la partida a multi (si no lo era) y devuelve su código de invitación. Solo el creador:
    // un miembro invitado recibe NotOwner, y alguien que no participa GameNotFound.
    EnableMultiplayerResponse enable_multiplayer(uint32_t game_id, const std::string& username);

    // El Game de la partida, que lanza si no estaba corriendo. Solo para un usuario conectado a
    // ella: el Game sigue vivo mientras quede algún conectado. Lanza std::runtime_error si lo
    // guardado no se puede leer (load_game y join_game ya lo lanzaron: solo pasa al crear).
    Inbox& attach(uint32_t game_id);

    bool is_running(uint32_t game_id) const;

    // El usuario se desconectó de la partida: puede volver a entrar con load_game. Si era el
    // último, el Game se detiene.
    void leave_game(uint32_t game_id, const std::string& username);

    ~GameRegistry();

private:
    const std::vector<WorldInfo> worlds;
    const std::map<std::string, stardew::maps::MapDefinition> maps;
    // Cupo de una partida cuando pasa a multi.
    const uint8_t max_players;
    GameStore& store;

    std::map<uint32_t, GameRecord> games;
    std::map<std::string, std::set<uint32_t>> games_by_user;
    std::map<std::string, uint32_t> games_by_code;
    std::map<uint32_t, std::set<std::string>> connected;
    std::mt19937 rng{std::random_device{}()};
    uint32_t next_game_id = 1;
    std::map<uint32_t, std::unique_ptr<Game>> running;
    // Detenidos que pueden seguir en su último tick, uno por partida; reap() los destruye cuando
    // terminan. Si su partida se vuelve a lanzar antes, launch() espera a que terminen: en ese
    // último tick pueden estar guardando, y el nuevo tiene que cargar lo que guardaron.
    std::map<uint32_t, std::unique_ptr<Game>> finished;
    mutable std::mutex mutex;

    bool world_exists(const std::string& world_id) const;
    std::string world_name(const std::string& world_id) const;
    void add(const GameRecord& game);
    // Las que siguen requieren tener el lock tomado.
    std::string new_invite_code();
    void reap();
    // Lanza el Game de la partida si no está corriendo, con lo que tenga guardado. false si lo
    // guardado no se puede leer.
    bool launch(uint32_t game_id);
    static bool is_member(const GameRecord& game, const std::string& username);
    static std::optional<uint32_t> player_id_of(const GameRecord& game,
                                                const std::string& username);
    static bool name_taken(const GameRecord& game, const std::string& character_name);
};

#endif  // GAME_REGISTRY_H
