#pragma once

#include <cstdint>
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

    // La version del protocolo la agrega la implementacion.
    virtual LobbyStatus login(const std::string& username) = 0;
    virtual std::vector<WorldInfo> listWorlds() = 0;
    virtual std::vector<SavedGameInfo> listMyGames() = 0;
    // Ok, InvalidInviteCode, GameFull o AlreadyMember (ya esta en esa partida: se entra con
    // loadGame desde sus partidas).
    virtual LobbyStatus checkInviteCode(const std::string& code) = 0;
    virtual GameEnteredResponse createGame(const CreateGameRequest& request) = 0;
    // Entrar por primera vez a una partida multi, con el codigo de invitacion.
    virtual GameEnteredResponse joinGame(const JoinGameRequest& request) = 0;
    // Retomar una partida de la que ya es miembro, con el personaje guardado.
    virtual GameEnteredResponse loadGame(uint32_t gameId) = 0;
    // abre la partida a otros y devuelve su codigo; solo el dueño (si no, NotOwner)
    virtual EnableMultiplayerResponse enableMultiplayer(uint32_t gameId) = 0;
};
