#ifndef LOBBY_SESSION_H
#define LOBBY_SESSION_H

#include <functional>
#include <map>
#include <optional>
#include <string>

#include "common/lib/thread.h"
#include "common/lobby_dto.h"
#include "common/net/socket.h"
#include "server/game/game_registry.h"
#include "server/net/server_protocol.h"

class LobbySession: public Thread {
public:
    LobbySession(Socket&& peer, GameRegistry& registry);

    void start() override;
    void run() override;

    // Corta la conexión con el cliente (desbloqueando el recv) y joinea el thread.
    ~LobbySession() override;

private:
    // Solo se joinea un thread que efectivamente arrancó.
    bool started = false;
    ServerProtocol protocol;
    GameRegistry& registry;
    // Vacío hasta que el login sea válido. Solo lo toca el thread de la sesión.
    std::optional<std::string> username;
    // La partida reservada con CREATE_GAME, JOIN_GAME o LOAD_GAME. Se suelta al cerrar la sesión.
    std::optional<uint32_t> current_game;

    using HandlerMap = std::map<LobbyOp, std::function<bool()>>;
    // Operaciones válidas después del login. LOGIN no está: va una sola vez por conexión.
    const HandlerMap handlers;

    // Cada una retorna false si hay que cerrar la conexión.
    bool handle_login();
    bool handle_next_operation();
    bool handle_list_worlds();
    bool handle_list_my_games();
    bool handle_check_invite_code();
    bool handle_create_game();
    bool handle_join_game();
    bool handle_load_game();
    bool handle_enable_multiplayer();
    // Responde el ingreso y, con Ok, juega. Con LeaveGame vuelve al lobby; si se corta la
    // conexión retorna false.
    bool enter_game(const GameEnteredResponse& response, const GameMember& member);
    // true si el jugador salió con LeaveGame y la conexión sigue abierta.
    bool play(uint32_t player_id, const GameMember& member);
    LobbyStatus validate(const LoginRequest& req) const;
};

#endif  // LOBBY_SESSION_H
