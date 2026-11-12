#include "server/net/sender.h"

#include <memory>
#include <stdexcept>
#include <utility>
#include <variant>

#include "common/logger.h"
#include "common/net/liberror.h"

Sender::Sender(ServerProtocol& protocol, std::shared_ptr<Outbox> outbox):
        protocol(protocol), outbox(std::move(outbox)) {}

void Sender::start() {
    Thread::start();
    started = true;
}

void Sender::run() {
    try {
        while (should_keep_running() && send(outbox->pop())) {}
    } catch (const ClosedQueue&) {
    } catch (const LibError& e) {
        Logger::debug("envío cortado: {}", e.what());
    }
}

bool Sender::send(const GameMessage& message) {
    if (const LocationData* location = std::get_if<LocationData>(&message)) {
        return protocol.send_location_data(*location);
    }
    if (const PlayerInfo* info = std::get_if<PlayerInfo>(&message)) {
        return protocol.send_player_info(*info);
    }
    return protocol.send_snapshot(std::get<Snapshot>(message));
}

void Sender::join() {
    Thread::join();
    started = false;  // El destructor no lo vuelve a joinear.
}

Sender::~Sender() {
    stop();
    try {
        outbox->close();
    } catch (const std::runtime_error&) {}
    if (started) {
        join();
    }
}
