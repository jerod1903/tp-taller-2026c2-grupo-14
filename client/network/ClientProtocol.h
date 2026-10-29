#pragma once

#include <cstdint>
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
    explicit ClientProtocol(Socket&& socket);

    ClientProtocol(const ClientProtocol&) = delete;
    ClientProtocol& operator=(const ClientProtocol&) = delete;

    LobbyStatus login(const std::string& username) override;
    std::vector<WorldInfo> listWorlds() override;
    std::vector<SavedGameInfo> listMyGames() override;
    LobbyStatus checkInviteCode(const std::string& code) override;
    CreateGameResponse createGame(const CreateGameRequest& request) override;
    JoinGameResponse joinGame(const JoinGameRequest& request) override;
    LoadGameResponse loadGame(uint32_t gameId) override;

    void shutdown();

private:
    // False si no se pudo mandar.
    bool sendOperation(LobbyOp op);
    bool sendAppearance(const CharacterAppearance& appearance);

    LobbyStatus recvStatus();
    CharacterAppearance recvAppearance();

    BaseProtocol baseProtocol;
};
