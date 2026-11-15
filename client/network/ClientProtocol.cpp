#include "client/network/ClientProtocol.h"

#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

// Formato de los mensajes (enteros en big endian, strings con largo u16 adelante,
// apariencia = hairStyle u16, hairColor u8, shirt u16, pants u16, hat u16):
//
//   Login:           op, version u8, username                    -> status u8
//   ListWorlds:      op                                          -> count u16, (worldId, name)*
//   ListMyGames:     op                                          -> count u16, (gameId u32,
//                    gameName, worldName, characterName, apariencia, isMultiplayer u8)*
//   CheckInviteCode: op, inviteCode                              -> status u8
//   CreateGame:      op, worldId, gameName, characterName, apariencia -> status u8, gameId u32
//   JoinGame:        op, inviteCode, characterName, apariencia   -> status u8, gameId u32
//   LoadGame:        op, gameId u32                              -> status u8
//   EnableMultiplayer: op, gameId u32                            -> status u8, inviteCode
//
// Durante la partida:
//   Acciones del jugador: tag GameOp u8 y su payload, ver la tabla de GameOp en common/game_dto.h
//   Mensajes del servidor, con su tag GameMsg u8 adelante:
//     LocationData:  worldId
//     Snapshot:      tick u32, count u16, (playerId u32, areaId, x u32, y u32, dirX u8, dirY u8,
//                    health u16, energy u16, gold u32)*,
//                    count u16, (entityId u32, type u8, areaId, x u32, y u32, dirX u8, dirY u8)*
//     GameEnded:     reason u8
//     PlayerInfo:    playerId u32, characterName, apariencia
//     InventoryState: selectedSlot u8, count u16, (item u8, quantity u8)*
//
// Es el mismo que ServerProtocol. Las respuestas mandan todos los campos aunque el status no sea
// Ok, asi el largo no depende del status.

namespace {
// Las operaciones de BaseProtocol devuelven false o std::nullopt si se corto la conexion: aca se
// convierte en excepcion, asi cada metodo se lee como la secuencia del mensaje.
void ensure(bool sent) {
    if (!sent) {
        throw LobbyConnectionError("no se pudo enviar al servidor");
    }
}

template <typename T>
T ensure(std::optional<T> received) {
    if (!received) {
        throw LobbyConnectionError("el servidor cerro la conexion");
    }
    return std::move(*received);
}
}  // namespace

ClientProtocol::ClientProtocol(Socket&& socket): baseProtocol(std::move(socket)) {}

LobbyStatus ClientProtocol::login(const std::string& username) {
    ensure(sendOperation(LobbyOp::Login) && baseProtocol.send_u8(PROTOCOL_VERSION) &&
           baseProtocol.send_string(username));
    return recvStatus();
}

std::vector<WorldInfo> ClientProtocol::listWorlds() {
    ensure(sendOperation(LobbyOp::ListWorlds));
    std::vector<WorldInfo> worlds(ensure(baseProtocol.recv_u16()));
    for (WorldInfo& world: worlds) {
        world.worldId = ensure(baseProtocol.recv_string());
        world.name = ensure(baseProtocol.recv_string());
    }
    return worlds;
}

std::vector<SavedGameInfo> ClientProtocol::listMyGames() {
    ensure(sendOperation(LobbyOp::ListMyGames));
    std::vector<SavedGameInfo> games(ensure(baseProtocol.recv_u16()));
    for (SavedGameInfo& game: games) {
        game.gameId = ensure(baseProtocol.recv_u32());
        game.gameName = ensure(baseProtocol.recv_string());
        game.worldName = ensure(baseProtocol.recv_string());
        game.characterName = ensure(baseProtocol.recv_string());
        game.appearance = recvAppearance();
        game.isMultiplayer = ensure(baseProtocol.recv_u8()) != 0;
    }
    return games;
}

LobbyStatus ClientProtocol::checkInviteCode(const std::string& code) {
    ensure(sendOperation(LobbyOp::CheckInviteCode) && baseProtocol.send_string(code));
    return recvStatus();
}

GameEnteredResponse ClientProtocol::createGame(const CreateGameRequest& request) {
    ensure(sendOperation(LobbyOp::CreateGame) && baseProtocol.send_string(request.worldId) &&
           baseProtocol.send_string(request.gameName) &&
           baseProtocol.send_string(request.characterName) && sendAppearance(request.appearance));
    return recvGameEntered();
}

GameEnteredResponse ClientProtocol::joinGame(const JoinGameRequest& request) {
    ensure(sendOperation(LobbyOp::JoinGame) && baseProtocol.send_string(request.inviteCode) &&
           baseProtocol.send_string(request.characterName) && sendAppearance(request.appearance));
    return recvGameEntered();
}

GameEnteredResponse ClientProtocol::loadGame(uint32_t gameId) {
    ensure(sendOperation(LobbyOp::LoadGame) && baseProtocol.send_u32(gameId));
    return recvGameEntered();
}

EnableMultiplayerResponse ClientProtocol::enableMultiplayer(uint32_t gameId) {
    ensure(sendOperation(LobbyOp::EnableMultiplayer) && baseProtocol.send_u32(gameId));
    EnableMultiplayerResponse response{};
    response.status = recvStatus();
    response.inviteCode = ensure(baseProtocol.recv_string());
    return response;
}

// Los ids van siempre, también con un status de error (en 0).
GameEnteredResponse ClientProtocol::recvGameEntered() {
    GameEnteredResponse response{};
    response.status = recvStatus();
    response.gameId = ensure(baseProtocol.recv_u32());
    response.playerId = ensure(baseProtocol.recv_u32());
    return response;
}

void ClientProtocol::sendMove(int8_t dir_x, int8_t dir_y) {
    ensure(sendGameOp(GameOp::Move) && baseProtocol.send_u8(static_cast<uint8_t>(dir_x)) &&
           baseProtocol.send_u8(static_cast<uint8_t>(dir_y)));
}

void ClientProtocol::sendLeaveGame() { ensure(sendGameOp(GameOp::LeaveGame)); }

void ClientProtocol::sendSelectSlot(uint8_t slot) {
    ensure(sendGameOp(GameOp::SelectSlot) && baseProtocol.send_u8(slot));
}


ServerGameMessage ClientProtocol::recvGameMessage() {
    auto msg = static_cast<GameMsg>(ensure(baseProtocol.recv_u8()));
    switch (msg) {
        case GameMsg::LocationData:
            return LocationData{ensure(baseProtocol.recv_string())};
        case GameMsg::Snapshot:
            return recvSnapshot();
        case GameMsg::GameEnded:
            return GameEnded{static_cast<EndReason>(ensure(baseProtocol.recv_u8()))};
        case GameMsg::PlayerInfo:
            return recvPlayerInfo();
        case GameMsg::InventoryState:
            return recvInventoryState();
        case GameMsg::ChatMessage:
            return recvChatMessage();
    }
    throw std::runtime_error("mensaje de partida desconocido");
}

void ClientProtocol::shutdown() { baseProtocol.shutdown(); }

bool ClientProtocol::sendOperation(LobbyOp op) {
    return baseProtocol.send_u8(static_cast<uint8_t>(op));
}

bool ClientProtocol::sendGameOp(GameOp op) {
    return baseProtocol.send_u8(static_cast<uint8_t>(op));
}

bool ClientProtocol::sendAppearance(const CharacterAppearance& appearance) {
    return baseProtocol.send_u16(appearance.hairStyle) &&
           baseProtocol.send_u8(appearance.hairColor) && baseProtocol.send_u16(appearance.shirt) &&
           baseProtocol.send_u16(appearance.pants) && baseProtocol.send_u16(appearance.hat);
}

LobbyStatus ClientProtocol::recvStatus() {
    return static_cast<LobbyStatus>(ensure(baseProtocol.recv_u8()));
}

CharacterAppearance ClientProtocol::recvAppearance() {
    CharacterAppearance appearance{};
    appearance.hairStyle = ensure(baseProtocol.recv_u16());
    appearance.hairColor = ensure(baseProtocol.recv_u8());
    appearance.shirt = ensure(baseProtocol.recv_u16());
    appearance.pants = ensure(baseProtocol.recv_u16());
    appearance.hat = ensure(baseProtocol.recv_u16());
    return appearance;
}

Snapshot ClientProtocol::recvSnapshot() {
    Snapshot snapshot{};
    snapshot.tick = ensure(baseProtocol.recv_u32());
    uint16_t count = ensure(baseProtocol.recv_u16());
    for (uint16_t i = 0; i < count; ++i) {
        snapshot.players.push_back(recvPlayerState());
    }
    uint16_t entity_count = ensure(baseProtocol.recv_u16());
    for (uint16_t i = 0; i < entity_count; ++i) {
        snapshot.entities.push_back(recvEntityState());
    }
    return snapshot;
}

PlayerState ClientProtocol::recvPlayerState() {
    PlayerState player{};
    player.playerId = ensure(baseProtocol.recv_u32());
    player.areaId = ensure(baseProtocol.recv_string());
    player.x = ensure(baseProtocol.recv_u32());
    player.y = ensure(baseProtocol.recv_u32());
    player.dirX = static_cast<int8_t>(ensure(baseProtocol.recv_u8()));
    player.dirY = static_cast<int8_t>(ensure(baseProtocol.recv_u8()));
    player.health = ensure(baseProtocol.recv_u16());
    player.energy = ensure(baseProtocol.recv_u16());
    player.gold = ensure(baseProtocol.recv_u32());
    return player;
}

EntityState ClientProtocol::recvEntityState() {
    EntityState entity{};
    entity.entityId = ensure(baseProtocol.recv_u32());
    entity.type = static_cast<EntityType>(ensure(baseProtocol.recv_u8()));
    entity.areaId = ensure(baseProtocol.recv_string());
    entity.x = ensure(baseProtocol.recv_u32());
    entity.y = ensure(baseProtocol.recv_u32());
    entity.dirX = static_cast<int8_t>(ensure(baseProtocol.recv_u8()));
    entity.dirY = static_cast<int8_t>(ensure(baseProtocol.recv_u8()));
    return entity;
}

PlayerInfo ClientProtocol::recvPlayerInfo() {
    PlayerInfo info{};
    info.playerId = ensure(baseProtocol.recv_u32());
    info.characterName = ensure(baseProtocol.recv_string());
    info.appearance = recvAppearance();
    return info;
}


void ClientProtocol::sendChat(const std::string& texto) {
    ensure(sendGameOp(GameOp::Chat) && baseProtocol.send_string(texto));
}

ChatMessage ClientProtocol::recvChatMessage() {
    ChatMessage mensaje{};
    mensaje.kind = static_cast<ChatKind>(ensure(baseProtocol.recv_u8()));
    mensaje.from = ensure(baseProtocol.recv_string());
    mensaje.text = ensure(baseProtocol.recv_string());
    return mensaje;
}

InventoryState ClientProtocol::recvInventoryState() {
    InventoryState inventory{};
    inventory.selectedSlot = ensure(baseProtocol.recv_u8());
    uint16_t count = ensure(baseProtocol.recv_u16());
    for (uint16_t i = 0; i < count; ++i) {
        InventorySlot slot{};
        slot.item = static_cast<ItemType>(ensure(baseProtocol.recv_u8()));
        slot.quantity = ensure(baseProtocol.recv_u8());
        inventory.slots.push_back(slot);
    }
    return inventory;
}
