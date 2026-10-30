#include "client/network/client_protocol.h"

#include "common/game_codec.h"
#include "common/net/socket.h"

ClientProtocol::ClientProtocol(const std::string& host, const std::string& service):
        baseProtocol(Socket(host.c_str(), service.c_str())) {}

bool ClientProtocol::send(const ClientMessage& msg) { return write_message(baseProtocol, msg); }

std::optional<ServerMessage> ClientProtocol::recv() { return read_server_message(baseProtocol); }

void ClientProtocol::shutdown() { baseProtocol.shutdown(); }