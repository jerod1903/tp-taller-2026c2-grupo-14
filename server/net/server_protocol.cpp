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

std::optional<EnableMultiplayerRequest> ServerProtocol::recv_enable_multiplayer() {
    std::optional<uint32_t> game_id = baseProtocol.recv_u32();
    if (!game_id)
        return std::nullopt;
    return EnableMultiplayerRequest{*game_id};
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
bool ServerProtocol::send_game_entered(const GameEnteredResponse& response) {
    return send_status(response.status) && baseProtocol.send_u32(response.gameId) &&
           baseProtocol.send_u32(response.playerId);
}

bool ServerProtocol::send_enable_multiplayer(const EnableMultiplayerResponse& response) {
    return send_status(response.status) && baseProtocol.send_string(response.inviteCode);
}


std::optional<GameOp> ServerProtocol::recv_game_op() {
    std::optional<uint8_t> op = baseProtocol.recv_u8();
    if (!op)
        return std::nullopt;
    return static_cast<GameOp>(*op);
}

// Cada eje viaja como u8: el -1 llega como 0xFF.
std::optional<MoveRequest> ServerProtocol::recv_move() {
    std::optional<uint8_t> dir_x = baseProtocol.recv_u8();
    std::optional<uint8_t> dir_y = dir_x ? baseProtocol.recv_u8() : std::nullopt;
    if (!dir_y)
        return std::nullopt;
    return MoveRequest{static_cast<int8_t>(*dir_x), static_cast<int8_t>(*dir_y)};
}


std::optional<SelectSlotRequest> ServerProtocol::recv_select_slot() {
    std::optional<uint8_t> slot = baseProtocol.recv_u8();
    if (!slot)
        return std::nullopt;
    return SelectSlotRequest{*slot};
}


std::optional<ChatRequest> ServerProtocol::recv_chat() {
    std::optional<std::string> text = baseProtocol.recv_string();
    if (!text)
        return std::nullopt;
    return ChatRequest{std::move(*text)};
}

bool ServerProtocol::send_location_data(const LocationData& location) {
    return send_game_msg(GameMsg::LocationData) && baseProtocol.send_string(location.worldId);
}

bool ServerProtocol::send_snapshot(const Snapshot& snapshot) {
    return send_game_msg(GameMsg::Snapshot) && baseProtocol.send_u32(snapshot.tick) &&
           send_count(snapshot.players.size()) &&
           std::all_of(snapshot.players.begin(), snapshot.players.end(),
                       [this](const PlayerState& player) { return send_player_state(player); }) &&
           send_count(snapshot.entities.size()) &&
           std::all_of(snapshot.entities.begin(), snapshot.entities.end(),
                       [this](const EntityState& entity) { return send_entity_state(entity); });
}

bool ServerProtocol::send_player_state(const PlayerState& player) {
    return baseProtocol.send_u32(player.playerId) && baseProtocol.send_string(player.areaId) &&
           baseProtocol.send_u32(player.x) && baseProtocol.send_u32(player.y) &&
           baseProtocol.send_u8(static_cast<uint8_t>(player.dirX)) &&
           baseProtocol.send_u8(static_cast<uint8_t>(player.dirY)) &&
           baseProtocol.send_u16(player.health) && baseProtocol.send_u16(player.energy) &&
           baseProtocol.send_u32(player.gold);
}

bool ServerProtocol::send_entity_state(const EntityState& entity) {
    return baseProtocol.send_u32(entity.entityId) &&
           baseProtocol.send_u8(static_cast<uint8_t>(entity.type)) &&
           baseProtocol.send_string(entity.areaId) && baseProtocol.send_u32(entity.x) &&
           baseProtocol.send_u32(entity.y) &&
           baseProtocol.send_u8(static_cast<uint8_t>(entity.dirX)) &&
           baseProtocol.send_u8(static_cast<uint8_t>(entity.dirY)) &&
           baseProtocol.send_u8(static_cast<uint8_t>(entity.emote));
}

bool ServerProtocol::send_player_info(const PlayerInfo& info) {
    return send_game_msg(GameMsg::PlayerInfo) && baseProtocol.send_u32(info.playerId) &&
           baseProtocol.send_string(info.characterName) && send_appearance(info.appearance);
}

bool ServerProtocol::send_inventory_state(const InventoryState& inventory) {
    return send_game_msg(GameMsg::InventoryState) && baseProtocol.send_u8(inventory.selectedSlot) &&
           send_count(inventory.slots.size()) &&
           std::all_of(inventory.slots.begin(), inventory.slots.end(),
                       [this](const InventorySlot& slot) {
                           return baseProtocol.send_u8(static_cast<uint8_t>(slot.item)) &&
                                  baseProtocol.send_u8(slot.quantity);
                       });
}


bool ServerProtocol::send_chat_message(const ChatMessage& message) {
    return send_game_msg(GameMsg::ChatMessage) &&
           baseProtocol.send_u8(static_cast<uint8_t>(message.kind)) &&
           baseProtocol.send_string(message.from) && baseProtocol.send_string(message.text);
}

bool ServerProtocol::send_game_ended(const GameEnded& ended) {
    return send_game_msg(GameMsg::GameEnded) &&
           baseProtocol.send_u8(static_cast<uint8_t>(ended.reason));
}

bool ServerProtocol::send_game_msg(GameMsg msg) {
    return baseProtocol.send_u8(static_cast<uint8_t>(msg));
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
