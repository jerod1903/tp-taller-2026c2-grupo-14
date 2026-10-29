#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "client/menu/backend/ILobbyProtocol.h"

// Servidor falso para desarrollar las pantallas sin red. Datos fijos:
//   - codigo "ABC123" es valido, "LLENA1" es de una partida llena, "MIA123" es de una
//     partida donde el jugador ya esta (la 2), cualquier otro es invalido.
//   - los nombres vacios o de mas de 20 caracteres se rechazan con InvalidName.
//   - el usuario "ocupado" ya esta conectado (AlreadyConnected).
class MockLobbyProtocol: public ILobbyProtocol {
public:
    MockLobbyProtocol();

    LobbyStatus login(const std::string& username) override;
    std::vector<WorldInfo> listWorlds() override;
    std::vector<SavedGameInfo> listMyGames() override;
    CheckInviteCodeResponse checkInviteCode(const std::string& code) override;
    CreateGameResponse createGame(const CreateGameRequest& request) override;
    JoinGameResponse joinGame(const JoinGameRequest& request) override;

private:
    std::vector<WorldInfo> worlds;
    std::vector<SavedGameInfo> savedGames;
    uint32_t nextGameId;
};
