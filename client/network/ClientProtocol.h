#pragma once

#include <cstdint>
#include <optional>
#include <string>
#include <vector>

#include "client/menu/backend/ILobbyProtocol.h"
#include "common/base_protocol.h"
#include "common/lobby_dto.h"
#include "common/net/socket.h"

// Lado cliente del protocolo del lobby: cada metodo manda una operacion y espera su respuesta.
// Si el servidor corta la conexion a mitad de una respuesta, tira LobbyConnectionError.
class ClientProtocol: public ILobbyProtocol {
public:
    // userId: el UUID v4 que el cliente generó y guardó; se manda en el login.
    ClientProtocol(Socket&& socket, const UserId& userId);

    ClientProtocol(const ClientProtocol&) = delete;
    ClientProtocol& operator=(const ClientProtocol&) = delete;

    LobbyStatus login(const std::string& username) override;
    std::vector<WorldInfo> listWorlds() override;
    std::vector<SavedGameInfo> listMyGames() override;
    CheckInviteCodeResponse checkInviteCode(const std::string& code) override;
    CreateGameResponse createGame(const CreateGameRequest& request) override;
    JoinGameResponse joinGame(const JoinGameRequest& request) override;

    void shutdown();

private:
    void sendOperation(LobbyOp op);
    void sendString(const std::string& value);
    void sendAppearance(const CharacterAppearance& appearance);

    LobbyStatus recvStatus();
    uint8_t recvU8();
    uint16_t recvU16();
    uint32_t recvU32();
    std::string recvString();
    CharacterAppearance recvAppearance();

    BaseProtocol baseProtocol;
    const UserId userId;
};
