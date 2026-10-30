#ifndef CLIENT_PROTOCOL_H
#define CLIENT_PROTOCOL_H

#include <optional>
#include <string>

#include "common/base_protocol.h"
#include "common/game_dto.h"

class ClientProtocol {
public:
    // Lanza una excepcion si no puede conectarse.
    ClientProtocol(const std::string& host, const std::string& service);

    // false si la conexion esta cerrada.
    bool send(const ClientMessage& msg);

    // nullopt si el servidor cerro la conexion.
    std::optional<ServerMessage> recv();

    // Despierta a un recv() bloqueado en otro hilo.
    void shutdown();

private:
    BaseProtocol baseProtocol;
};

#endif  // CLIENT_PROTOCOL_H