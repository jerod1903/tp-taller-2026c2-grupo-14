#pragma once

#include <stdexcept>
#include <string>
#include <vector>

#include "common/lobby_dto.h"

// Se corto la conexion con el servidor a mitad de una operacion.
class LobbyConnectionError: public std::runtime_error {
public:
    using std::runtime_error::runtime_error;
};

// Lo que el menu necesita del servidor, un metodo por mensaje del lobby.
// Las pantallas solo conocen esta interfaz: hoy la implementa MockLobbyProtocol,
// despues la implementara ClientProtocol sin que las pantallas cambien.
// Cualquier metodo puede tirar LobbyConnectionError.
class ILobbyProtocol {
public:
    virtual ~ILobbyProtocol() = default;

    // La version del protocolo y el userId los agrega la implementacion.
    virtual LobbyStatus login(const std::string& username) = 0;
    virtual std::vector<WorldInfo> listWorlds() = 0;
    virtual std::vector<SavedGameInfo> listMyGames() = 0;
    virtual CheckInviteCodeResponse checkInviteCode(const std::string& code) = 0;
    virtual CreateGameResponse createGame(const CreateGameRequest& request) = 0;
    // Sirve para unirse a una partida nueva y para retomar una propia (por gameId).
    virtual JoinGameResponse joinGame(const JoinGameRequest& request) = 0;
};
