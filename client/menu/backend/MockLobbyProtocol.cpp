#include "client/menu/backend/MockLobbyProtocol.h"

#include <algorithm>
#include <string>
#include <vector>

namespace {
constexpr std::size_t MAX_NAME_LENGTH = 20;
const char* const VALID_INVITE_CODE = "ABC123";
const char* const FULL_GAME_INVITE_CODE = "LLENA1";
const char* const OWN_GAME_INVITE_CODE = "MIA123";
const char* const CONNECTED_USERNAME = "ocupado";
// Partida a la que lleva VALID_INVITE_CODE.
constexpr uint32_t INVITE_GAME_ID = 50;
// Partida de savedGames a la que lleva OWN_GAME_INVITE_CODE.
constexpr uint32_t OWN_GAME_ID = 2;
constexpr uint32_t FIRST_CREATED_GAME_ID = 100;

bool isValidName(const std::string& name) {
    return !name.empty() && name.size() <= MAX_NAME_LENGTH;
}
}  // namespace

MockLobbyProtocol::MockLobbyProtocol():
        worlds{
                {"valle_clasico", "Valle Clásico"},
                {"costa_brumosa", "Costa Brumosa"},
                {"bosque_cerrado", "Bosque Cerrado"},
        },
        savedGames{
                {1, "Granja de Martu", "Martu", {3, 2, 10, 1, 0}, false},
                {OWN_GAME_ID, "Partida con los chicos", "Pandamos", {7, 5, 22, 3, 4}, true},
                {3, "Prueba de mazmorra", "dipa", {1, 9, 5, 2, 0}, true},
        },
        nextGameId(FIRST_CREATED_GAME_ID) {}

LobbyStatus MockLobbyProtocol::login(const std::string& username) {
    if (!isValidName(username)) {
        return LobbyStatus::InvalidName;
    }
    return username == CONNECTED_USERNAME ? LobbyStatus::AlreadyConnected : LobbyStatus::Ok;
}

std::vector<WorldInfo> MockLobbyProtocol::listWorlds() { return worlds; }

std::vector<SavedGameInfo> MockLobbyProtocol::listMyGames() { return savedGames; }

CheckInviteCodeResponse MockLobbyProtocol::checkInviteCode(const std::string& code) {
    if (code == OWN_GAME_INVITE_CODE) {
        return {LobbyStatus::Ok, true, OWN_GAME_ID};
    }
    if (code == VALID_INVITE_CODE) {
        // El DTO solo garantiza gameId si alreadyMember; el mock lo manda igual para que
        // el flujo de unirse tenga a que partida apuntar hasta que se defina con el server.
        return {LobbyStatus::Ok, false, INVITE_GAME_ID};
    }
    if (code == FULL_GAME_INVITE_CODE) {
        return {LobbyStatus::GameFull, false, 0};
    }
    return {LobbyStatus::InvalidInviteCode, false, 0};
}

CreateGameResponse MockLobbyProtocol::createGame(const CreateGameRequest& request) {
    if (!isValidName(request.gameName) || !isValidName(request.characterName)) {
        return {LobbyStatus::InvalidName, 0};
    }
    bool worldExists = std::any_of(worlds.begin(), worlds.end(), [&](const WorldInfo& world) {
        return world.worldId == request.worldId;
    });
    if (!worldExists) {
        return {LobbyStatus::WorldNotFound, 0};
    }
    return {LobbyStatus::Ok, nextGameId++};
}

JoinGameResponse MockLobbyProtocol::joinGame(const JoinGameRequest& request) {
    if (!isValidName(request.characterName)) {
        return {LobbyStatus::InvalidName};
    }
    bool isSaved =
            std::any_of(savedGames.begin(), savedGames.end(),
                        [&](const SavedGameInfo& game) { return game.gameId == request.gameId; });
    bool isCreated = request.gameId >= FIRST_CREATED_GAME_ID && request.gameId < nextGameId;
    if (!isSaved && !isCreated && request.gameId != INVITE_GAME_ID) {
        return {LobbyStatus::GameNotFound};
    }
    return {LobbyStatus::Ok};
}
