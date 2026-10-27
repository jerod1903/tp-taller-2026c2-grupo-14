#ifndef ACCEPTOR_H
#define ACCEPTOR_H

#include <list>
#include <memory>

#include "common/lib/thread.h"
#include "common/net/socket.h"
#include "server/game/game_registry.h"
#include "server/lobby/lobby_session.h"

class Acceptor: public Thread {
public:
    Acceptor(const char* servicename, GameRegistry& registry);

    void start() override;
    void run() override;

    // Cierra el socket aceptador (desbloqueando el accept) y joinea el thread.
    ~Acceptor() override;

private:
    // Solo se joinea un thread que efectivamente arrancó.
    bool started = false;
    Socket acceptor;
    GameRegistry& registry;
    std::list<std::unique_ptr<LobbySession>> sessions;

    void clean();
};

#endif  // ACCEPTOR_H
