#include "client/network/ClientProtocol.h"

#include <utility>

// Formato de los mensajes (enteros en big endian, strings con largo u16 adelante,
// apariencia = hairStyle u16, hairColor u8, shirt u16, pants u16, hat u16):
//
//   Login:           op, version u8, userId 16 bytes, username   -> status u8
//   ListWorlds:      op                                          -> count u16, (worldId, name)*
//   ListMyGames:     op                                          -> count u16, (gameId u32,
//                    gameName, characterName, apariencia, isMultiplayer u8)*
//   CheckInviteCode: op, inviteCode                              -> status u8,
//                                                                   alreadyMember u8, gameId u32
//   CreateGame:      op, worldId, gameName, characterName, apariencia -> status u8, gameId u32
//   JoinGame:        op, gameId u32, characterName, apariencia   -> status u8
//
// Las tres ultimas son una propuesta: ServerProtocol todavia no las implementa. Las respuestas
// mandan todos los campos aunque el status no sea Ok, asi el largo no depende del status.

ClientProtocol::ClientProtocol(Socket&& socket, const UserId& userId):
        baseProtocol(std::move(socket)), userId(userId) {}

LobbyStatus ClientProtocol::login(const std::string& username) {
    sendOperation(LobbyOp::Login);
    if (!baseProtocol.send_u8(PROTOCOL_VERSION) ||
        !baseProtocol.send_bytes(userId.data(), userId.size())) {
        throw LobbyConnectionError("no se pudo enviar el login");
    }
    sendString(username);
    return recvStatus();
}

std::vector<WorldInfo> ClientProtocol::listWorlds() {
    sendOperation(LobbyOp::ListWorlds);
    std::vector<WorldInfo> worlds(recvU16());
    for (WorldInfo& world: worlds) {
        world.worldId = recvString();
        world.name = recvString();
    }
    return worlds;
}

std::vector<SavedGameInfo> ClientProtocol::listMyGames() {
    sendOperation(LobbyOp::ListMyGames);
    std::vector<SavedGameInfo> games(recvU16());
    for (SavedGameInfo& game: games) {
        game.gameId = recvU32();
        game.gameName = recvString();
        game.characterName = recvString();
        game.appearance = recvAppearance();
        game.isMultiplayer = recvU8() != 0;
    }
    return games;
}

CheckInviteCodeResponse ClientProtocol::checkInviteCode(const std::string& code) {
    sendOperation(LobbyOp::CheckInviteCode);
    sendString(code);
    CheckInviteCodeResponse response{};
    response.status = recvStatus();
    response.alreadyMember = recvU8() != 0;
    response.gameId = recvU32();
    return response;
}

CreateGameResponse ClientProtocol::createGame(const CreateGameRequest& request) {
    sendOperation(LobbyOp::CreateGame);
    sendString(request.worldId);
    sendString(request.gameName);
    sendString(request.characterName);
    sendAppearance(request.appearance);
    CreateGameResponse response{};
    response.status = recvStatus();
    response.gameId = recvU32();
    return response;
}

JoinGameResponse ClientProtocol::joinGame(const JoinGameRequest& request) {
    sendOperation(LobbyOp::JoinGame);
    if (!baseProtocol.send_u32(request.gameId)) {
        throw LobbyConnectionError("no se pudo enviar el pedido para unirse");
    }
    sendString(request.characterName);
    sendAppearance(request.appearance);
    return {recvStatus()};
}

void ClientProtocol::shutdown() { baseProtocol.shutdown(); }

void ClientProtocol::sendOperation(LobbyOp op) {
    if (!baseProtocol.send_u8(static_cast<uint8_t>(op))) {
        throw LobbyConnectionError("no se pudo enviar la operacion al servidor");
    }
}

void ClientProtocol::sendString(const std::string& value) {
    if (!baseProtocol.send_string(value)) {
        throw LobbyConnectionError("no se pudo enviar un texto al servidor");
    }
}

void ClientProtocol::sendAppearance(const CharacterAppearance& appearance) {
    bool sent = baseProtocol.send_u16(appearance.hairStyle) &&
                baseProtocol.send_u8(appearance.hairColor) &&
                baseProtocol.send_u16(appearance.shirt) &&
                baseProtocol.send_u16(appearance.pants) && baseProtocol.send_u16(appearance.hat);
    if (!sent) {
        throw LobbyConnectionError("no se pudo enviar la apariencia al servidor");
    }
}

LobbyStatus ClientProtocol::recvStatus() { return static_cast<LobbyStatus>(recvU8()); }

uint8_t ClientProtocol::recvU8() {
    std::optional<uint8_t> value = baseProtocol.recv_u8();
    if (!value) {
        throw LobbyConnectionError("el servidor cerro la conexion");
    }
    return *value;
}

uint16_t ClientProtocol::recvU16() {
    std::optional<uint16_t> value = baseProtocol.recv_u16();
    if (!value) {
        throw LobbyConnectionError("el servidor cerro la conexion");
    }
    return *value;
}

uint32_t ClientProtocol::recvU32() {
    std::optional<uint32_t> value = baseProtocol.recv_u32();
    if (!value) {
        throw LobbyConnectionError("el servidor cerro la conexion");
    }
    return *value;
}

std::string ClientProtocol::recvString() {
    std::optional<std::string> value = baseProtocol.recv_string();
    if (!value) {
        throw LobbyConnectionError("el servidor cerro la conexion");
    }
    return std::move(*value);
}

CharacterAppearance ClientProtocol::recvAppearance() {
    CharacterAppearance appearance{};
    appearance.hairStyle = recvU16();
    appearance.hairColor = recvU8();
    appearance.shirt = recvU16();
    appearance.pants = recvU16();
    appearance.hat = recvU16();
    return appearance;
}
