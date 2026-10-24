#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "client/menu/backend/LobbyTypes.h"

// Lo que el menu necesita del servidor, un metodo por mensaje del lobby.
// Las pantallas solo conocen esta interfaz: hoy la implementa MockLobbyProtocol,
// despues la implementara ClientProtocol sin que las pantallas cambien.
class ILobbyProtocol {
public:
    virtual ~ILobbyProtocol() = default;

    virtual LobbyStatus login(const std::string& username) = 0;
    virtual std::vector<WorldInfo> listWorlds() = 0;
    virtual std::vector<SavedGameInfo> listMyGames() = 0;
    virtual LobbyStatus checkInviteCode(const std::string& code) = 0;
    virtual GameEnteredResponse createGame(const CreateGameRequest& request) = 0;
    virtual GameEnteredResponse joinGame(const JoinGameRequest& request) = 0;
    virtual GameEnteredResponse loadGame(uint32_t gameId) = 0;
};
