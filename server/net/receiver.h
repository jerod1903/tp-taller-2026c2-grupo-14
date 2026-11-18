#ifndef RECEIVER_H
#define RECEIVER_H

#include <cstdint>
#include <functional>

#include "common/lib/thread.h"
#include "server/game/commands/command_factory.h"
#include "server/game/game_event.h"
#include "server/net/server_protocol.h"

// Al terminar, por LeaveGame o por un corte de la conexión, encola un LeaveCommand. Invite no va
// al Game: lo resuelve on_invite, que conoce la partida y el usuario.
class Receiver: public Thread {
public:
    Receiver(ServerProtocol& protocol, Inbox& inbox, uint32_t player_id,
             std::function<void()> on_invite = [] {});

    void start() override;
    void run() override;
    void join() override;

    // Si terminó por un LeaveGame y no por un corte de la conexión. Válido después del join.
    bool has_left() const;

    // Si puede estar bloqueado en el recv, cortar la conexión antes de destruirlo.
    ~Receiver() override;

private:
    bool started = false;
    ServerProtocol& protocol;
    CommandFactory factory;
    Inbox& inbox;
    const uint32_t player_id;
    std::function<void()> on_invite;
    bool left = false;
};

#endif  // RECEIVER_H
