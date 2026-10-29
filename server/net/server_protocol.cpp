#include "server/net/server_protocol.h"

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
    std::optional<std::string> username = baseProtocol.recv_string();
    if (!username)
        return std::nullopt;
    return LoginRequest{*version, std::move(*username)};
}

std::optional<CheckInviteCodeRequest> ServerProtocol::recv_check_invite_code() {
    std::optional<std::string> code = baseProtocol.recv_string();
    if (!code)
        return std::nullopt;
    return CheckInviteCodeRequest{std::move(*code)};
}

std::optional<CreateGameRequest> ServerProtocol::recv_create_game() {
    std::optional<std::string> world_id = baseProtocol.recv_string();
    if (!world_id)
        return std::nullopt;
    std::optional<std::string> game_name = baseProtocol.recv_string();
    if (!game_name)
        return std::nullopt;
    std::optional<std::string> character_name = baseProtocol.recv_string();
    if (!character_name)
        return std::nullopt;
    std::optional<CharacterAppearance> appearance = recv_appearance();
    if (!appearance)
        return std::nullopt;
    return CreateGameRequest{std::move(*world_id), std::move(*game_name),
                             std::move(*character_name), *appearance};
}

std::optional<JoinGameRequest> ServerProtocol::recv_join_game() {
    std::optional<std::string> code = baseProtocol.recv_string();
    if (!code)
        return std::nullopt;
    std::optional<std::string> character_name = baseProtocol.recv_string();
    if (!character_name)
        return std::nullopt;
    std::optional<CharacterAppearance> appearance = recv_appearance();
    if (!appearance)
        return std::nullopt;
    return JoinGameRequest{std::move(*code), std::move(*character_name), *appearance};
}

std::optional<LoadGameRequest> ServerProtocol::recv_load_game() {
    std::optional<uint32_t> game_id = baseProtocol.recv_u32();
    if (!game_id)
        return std::nullopt;
    return LoadGameRequest{*game_id};
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
        bool sent = baseProtocol.send_u32(game.gameId) && baseProtocol.send_string(game.gameName) &&
                    baseProtocol.send_string(game.worldName) &&
                    baseProtocol.send_string(game.characterName) &&
                    send_appearance(game.appearance) &&
                    baseProtocol.send_u8(game.isMultiplayer ? 1 : 0);
        if (!sent)
            return false;
    }
    return true;
}

// Los campos van siempre, aunque el status no sea Ok: el cliente lee una cantidad fija de bytes.
bool ServerProtocol::send_check_invite_code(const CheckInviteCodeResponse& response) {
    return send_status(response.status) && baseProtocol.send_u8(response.alreadyMember ? 1 : 0) &&
           baseProtocol.send_u32(response.gameId);
}

bool ServerProtocol::send_create_game(const CreateGameResponse& response) {
    return send_status(response.status) && baseProtocol.send_u32(response.gameId);
}

bool ServerProtocol::send_join_game(const JoinGameResponse& response) {
    return send_status(response.status) && baseProtocol.send_u32(response.gameId);
}

bool ServerProtocol::send_load_game(const LoadGameResponse& response) {
    return send_status(response.status);
}

bool ServerProtocol::send_count(std::size_t count) {
    return baseProtocol.send_u16(static_cast<uint16_t>(count));
}

bool ServerProtocol::send_appearance(const CharacterAppearance& appearance) {
    return baseProtocol.send_u16(appearance.hairStyle) &&
           baseProtocol.send_u8(appearance.hairColor) && baseProtocol.send_u16(appearance.shirt) &&
           baseProtocol.send_u16(appearance.pants) && baseProtocol.send_u16(appearance.hat);
}

std::optional<CharacterAppearance> ServerProtocol::recv_appearance() {
    std::optional<uint16_t> hair_style = baseProtocol.recv_u16();
    std::optional<uint8_t> hair_color = hair_style ? baseProtocol.recv_u8() : std::nullopt;
    std::optional<uint16_t> shirt = hair_color ? baseProtocol.recv_u16() : std::nullopt;
    std::optional<uint16_t> pants = shirt ? baseProtocol.recv_u16() : std::nullopt;
    std::optional<uint16_t> hat = pants ? baseProtocol.recv_u16() : std::nullopt;
    if (!hat)
        return std::nullopt;
    return CharacterAppearance{*hair_style, *hair_color, *shirt, *pants, *hat};
}

void ServerProtocol::shutdown() { baseProtocol.shutdown(); }
