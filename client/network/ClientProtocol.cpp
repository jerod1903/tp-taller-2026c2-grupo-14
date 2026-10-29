#include "client/network/ClientProtocol.h"

#include <optional>
#include <utility>

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

CreateGameResponse ClientProtocol::createGame(const CreateGameRequest& request) {
    ensure(sendOperation(LobbyOp::CreateGame) && baseProtocol.send_string(request.worldId) &&
           baseProtocol.send_string(request.gameName) &&
           baseProtocol.send_string(request.characterName) && sendAppearance(request.appearance));
    CreateGameResponse response{};
    response.status = recvStatus();
    response.gameId = ensure(baseProtocol.recv_u32());
    return response;
}

JoinGameResponse ClientProtocol::joinGame(const JoinGameRequest& request) {
    ensure(sendOperation(LobbyOp::JoinGame) && baseProtocol.send_string(request.inviteCode) &&
           baseProtocol.send_string(request.characterName) && sendAppearance(request.appearance));
    JoinGameResponse response{};
    response.status = recvStatus();
    response.gameId = ensure(baseProtocol.recv_u32());
    return response;
}

LoadGameResponse ClientProtocol::loadGame(uint32_t gameId) {
    ensure(sendOperation(LobbyOp::LoadGame) && baseProtocol.send_u32(gameId));
    return {recvStatus()};
}

void ClientProtocol::shutdown() { baseProtocol.shutdown(); }

bool ClientProtocol::sendOperation(LobbyOp op) {
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
