#ifndef GAME_CODEC_H
#define GAME_CODEC_H

#include <optional>

#include "common/base_protocol.h"
#include "common/game_dto.h"

// write_message: false si la conexion esta cerrada.
// read_*_message: nullopt si el otro extremo cerro; lanza std::runtime_error
// si llegan bytes invalidos.
bool write_message(BaseProtocol& protocol, const ClientMessage& msg);
bool write_message(BaseProtocol& protocol, const ServerMessage& msg);

std::optional<ClientMessage> read_client_message(BaseProtocol& protocol);
std::optional<ServerMessage> read_server_message(BaseProtocol& protocol);

#endif  // GAME_CODEC_H