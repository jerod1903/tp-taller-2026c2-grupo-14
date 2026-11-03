#include "recibidor.h"

#include <variant>

#include "common/net/liberror.h"

Recibidor::Recibidor(ClientProtocol& protocolo): protocolo(protocolo) { start(); }

void Recibidor::run() {
    try {
        bool termino = false;
        while (should_keep_running() && !termino) {
            ServerGameMessage mensaje = protocolo.recvGameMessage();
            // Lo que llega despues de GameEnded ya es del lobby
            termino = std::holds_alternative<GameEnded>(mensaje);
            mensajes.push(mensaje);
        }
    } catch (const LobbyConnectionError&) {
        // El servidor cerro la conexion, o se corto al cerrar el juego
    } catch (const LibError&) {
        // Error del socket al cortar la conexion
    }
}

std::optional<ServerGameMessage> Recibidor::recibir() {
    ServerGameMessage mensaje;
    if (!mensajes.try_pop(mensaje)) {
        return std::nullopt;
    }
    return mensaje;
}

Recibidor::~Recibidor() {
    stop();
    if (is_alive()) {
        protocolo.shutdown();
    }
    join();
}
