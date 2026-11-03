#ifndef SENDER_H
#define SENDER_H

#include <memory>

#include "common/lib/thread.h"
#include "server/game/game_event.h"
#include "server/net/server_protocol.h"

class Sender: public Thread {
public:
    Sender(ServerProtocol& protocol, std::shared_ptr<Outbox> outbox);

    void start() override;
    void run() override;
    void join() override;

    // Si puede estar bloqueado en un send, cortar la conexión antes de destruirlo.
    ~Sender() override;

private:
    bool started = false;
    ServerProtocol& protocol;
    std::shared_ptr<Outbox> outbox;

    bool send(const GameMessage& message);
};

#endif  // SENDER_H
