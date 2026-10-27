#include "server/net/acceptor.h"

#include <list>
#include <memory>
#include <utility>

#include <sys/socket.h>

#include "common/logger.h"
#include "common/net/liberror.h"

Acceptor::Acceptor(const char* servicename, GameRegistry& registry):
        acceptor(servicename), registry(registry) {
    Logger::info("escuchando en {}", servicename);
}

void Acceptor::start() {
    Thread::start();
    started = true;
}

void Acceptor::run() {
    try {
        while (should_keep_running()) {
            Socket peer = acceptor.accept();
            clean();
            sessions.emplace_back(std::make_unique<LobbySession>(std::move(peer), registry));
            sessions.back()->start();
            Logger::debug("conexión aceptada ({} sesiones)", sessions.size());
        }
    } catch (const LibError& e) {
        if (should_keep_running()) {
            Logger::error("accept falló: {}", e.what());
        }
    }
}

void Acceptor::clean() {
    for (std::list<std::unique_ptr<LobbySession>>::iterator it = sessions.begin();
         it != sessions.end();) {
        if (!(*it)->is_alive()) {
            it = sessions.erase(it);
        } else {
            ++it;
        }
    }
}

Acceptor::~Acceptor() {
    stop();
    try {
        acceptor.shutdown(SHUT_RDWR);
    } catch (...) {}
    if (started) {
        join();
    }
}
