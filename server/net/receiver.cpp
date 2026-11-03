#include "server/net/receiver.h"

#include <optional>

#include "common/logger.h"
#include "common/net/liberror.h"

Receiver::Receiver(ServerProtocol& protocol, Inbox& inbox, uint32_t player_id):
        protocol(protocol), inbox(inbox), player_id(player_id) {}

void Receiver::start() {
    Thread::start();
    started = true;
}

void Receiver::run() {
    try {
        std::optional<Request> request;
        while (should_keep_running() && (request = protocol.recv_request())) {
            inbox.push(PlayerAction{player_id, *request});
        }
    } catch (const LibError& e) {
        Logger::debug("conexión cortada: {}", e.what());
    } catch (const ClosedQueue&) {
        return;  // El Game se cerró: no hay a quién avisar.
    }
    try {
        inbox.push(PlayerLeft{player_id});
    } catch (const ClosedQueue&) {}
}

Receiver::~Receiver() {
    stop();
    if (started) {
        join();
    }
}
