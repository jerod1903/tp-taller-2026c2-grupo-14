#include "server/net/server_protocol.h"
#include "common/game_codec.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

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

bool ServerProtocol::send_world_list(const std::vector<WorldInfo>& worlds) {
    return send_count(worlds.size()) &&
           std::all_of(worlds.begin(), worlds.end(), [this](const WorldInfo& world) {
               return baseProtocol.send_string(world.worldId) &&
                      baseProtocol.send_string(world.name);
           });
}

bool ServerProtocol::send_saved_game_list(const std::vector<SavedGameInfo>& games) {
    if (!send_count(games.size()))
        return false;
    for (const SavedGameInfo& game: games) {
        bool sent =
                baseProtocol.send_u32(game.gameId) && baseProtocol.send_string(game.gameName) &&
                baseProtocol.send_string(game.characterName) && send_appearance(game.appearance) &&
                baseProtocol.send_u8(game.isMultiplayer ? 1 : 0) &&
                baseProtocol.send_u8(game.memberCount) &&
                baseProtocol.send_u8(game.connectedCount) && baseProtocol.send_u8(game.maxPlayers);
        if (!sent)
            return false;
    }
    return true;
}

bool ServerProtocol::send_count(std::size_t count) {
    return baseProtocol.send_u16(static_cast<uint16_t>(count));
}

bool ServerProtocol::send_appearance(const CharacterAppearance& appearance) {
    return baseProtocol.send_u16(appearance.hairStyle) &&
           baseProtocol.send_u8(appearance.hairColor) && baseProtocol.send_u16(appearance.shirt) &&
           baseProtocol.send_u16(appearance.pants) && baseProtocol.send_u16(appearance.hat);
}

void ServerProtocol::shutdown() { baseProtocol.shutdown(); }



std::optional<ClientMessage> ServerProtocol::recv_game_message() {
    return read_client_message(baseProtocol);
}

bool ServerProtocol::send_game_message(const ServerMessage& msg) {
    return write_message(baseProtocol, msg);
}