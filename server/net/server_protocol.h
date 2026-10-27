#ifndef SERVER_PROTOCOL_H
#define SERVER_PROTOCOL_H

#include <optional>
#include <vector>

#include "common/base_protocol.h"
#include "common/lobby_dto.h"
#include "common/net/socket.h"

class ServerProtocol {
public:
    explicit ServerProtocol(Socket&& socket);

    ServerProtocol(const ServerProtocol&) = delete;
    ServerProtocol& operator=(const ServerProtocol&) = delete;
    ServerProtocol(ServerProtocol&&) = default;
    ServerProtocol& operator=(ServerProtocol&&) = default;

    // std::nullopt si el peer cerró.
    std::optional<LobbyOp> recv_operation();
    std::optional<LoginRequest> recv_login();

    bool send_status(LobbyStatus status);
    bool send_world_list(const std::vector<WorldInfo>& worlds);
    bool send_saved_game_list(const std::vector<SavedGameInfo>& games);

    void shutdown();

private:
    BaseProtocol baseProtocol;

    bool send_count(std::size_t count);
    bool send_appearance(const CharacterAppearance& appearance);
};

#endif  // SERVER_PROTOCOL_H
