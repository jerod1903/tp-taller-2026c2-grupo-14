#pragma once

#include <cstdint>
#include <string>
#include <vector>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/network/IGameProtocol.h"
#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/lobby_dto.h"
#include "common/net/socket.h"

// Lado cliente del protocolo: el del lobby (cada metodo manda una operacion y espera su respuesta)
// y el de la partida. Si el servidor corta la conexion a mitad de un mensaje, tira
// LobbyConnectionError.
class ClientProtocol: public ILobbyProtocol, public IGameProtocol {
public:
    explicit ClientProtocol(Socket&& socket);

    ClientProtocol(const ClientProtocol&) = delete;
    ClientProtocol& operator=(const ClientProtocol&) = delete;

    LobbyStatus login(const std::string& username) override;
    std::vector<WorldInfo> listWorlds() override;
    std::vector<SavedGameInfo> listMyGames() override;
    LobbyStatus checkInviteCode(const std::string& code) override;
    GameEnteredResponse createGame(const CreateGameRequest& request) override;
    GameEnteredResponse joinGame(const JoinGameRequest& request) override;
    GameEnteredResponse loadGame(uint32_t gameId) override;
    EnableMultiplayerResponse enableMultiplayer(uint32_t gameId) override;

    void sendMove(int8_t dir_x, int8_t dir_y) override;
    void sendLeaveGame() override;
    void sendSelectSlot(uint8_t slot) override;
    void sendChat(const std::string& texto) override;
    void sendInteract() override;
    ServerGameMessage recvGameMessage() override;

    void shutdown() override;

private:
    // False si no se pudo mandar.
    bool sendOperation(LobbyOp op);
    bool sendAppearance(const CharacterAppearance& appearance);
    bool sendGameOp(GameOp op);

    LobbyStatus recvStatus();
    GameEnteredResponse recvGameEntered();
    CharacterAppearance recvAppearance();
    Snapshot recvSnapshot();
    PlayerState recvPlayerState();
    EntityState recvEntityState();
    PlayerInfo recvPlayerInfo();
    InventoryState recvInventoryState();
    ChatMessage recvChatMessage();

    BaseProtocol baseProtocol;
};
