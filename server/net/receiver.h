#ifndef RECEIVER_H
#define RECEIVER_H

#include <cstdint>

#include "common/lib/thread.h"
#include "server/game/game_event.h"
#include "server/net/server_protocol.h"

// Al cortarse la conexión avisa PlayerLeft al Game.
class Receiver: public Thread {
public:
    Receiver(ServerProtocol& protocol, Inbox& inbox, uint32_t player_id);

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
    Inbox& inbox;
    const uint32_t player_id;
    bool left = false;
};

#endif  // RECEIVER_H
