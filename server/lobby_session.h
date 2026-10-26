#ifndef LOBBY_SESSION_H
#define LOBBY_SESSION_H

#include <optional>
#include <string>

#include "common/lib/thread.h"
#include "common/lobby_dto.h"
#include "common/net/socket.h"

#include "server_protocol.h"

struct SessionIdentity {
    UserId userId;
    std::string username;
};

class LobbySession: public Thread {
public:
    explicit LobbySession(Socket&& peer);

    void start() override;
    void run() override;

    // Corta la conexión con el cliente (desbloqueando el recv) y joinea el thread.
    ~LobbySession() override;

private:
    // Solo se joinea un thread que efectivamente arrancó.
    bool started = false;
    ServerProtocol protocol;
    // Vacía hasta que el login sea válido. Solo la toca el thread de la sesión.
    std::optional<SessionIdentity> identity;

    // Retorna false si hay que cerrar la conexión.
    bool handle_login();
    LobbyStatus validate(const LoginRequest& req) const;
};

#endif  // LOBBY_SESSION_H
