#include "server_protocol.h"

#include <string>
#include <utility>

ServerProtocol::ServerProtocol(Socket&& socket): baseProtocol(std::move(socket)) {}

std::optional<LobbyOp> ServerProtocol::recv_operation() {
    std::optional<uint8_t> operation = baseProtocol.recv_u8();
    if (!operation) {
        return std::nullopt;
    }
    return static_cast<LobbyOp>(*operation);
}

std::optional<LoginRequest> ServerProtocol::recv_login() {
    std::optional<uint8_t> version = baseProtocol.recv_u8();
    if (!version)
        return std::nullopt;
    UserId user_id;
    if (!baseProtocol.recv_bytes(user_id.data(), user_id.size()))
        return std::nullopt;
    std::optional<std::string> username = baseProtocol.recv_string();
    if (!username)
        return std::nullopt;
    return LoginRequest{*version, user_id, std::move(*username)};
}

bool ServerProtocol::send_status(LobbyStatus status) {
    return baseProtocol.send_u8(static_cast<uint8_t>(status));
}

void ServerProtocol::shutdown() { baseProtocol.shutdown(); }
