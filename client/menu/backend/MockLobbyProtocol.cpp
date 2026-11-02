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
                {1, "Granja de prueba", "Valle Clásico", "Granjero", {3, 2, 6, 1, 0}, false},
                {2, "Partida con los chicos", "Costa Brumosa", "Pandamos", {7, 5, 2, 3, 4}, true},
                {3, "Prueba de mazmorra", "Bosque Cerrado", "dipa", {1, 9, 5, 2, 0}, true},
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

LobbyStatus MockLobbyProtocol::checkInviteCode(const std::string& code) {
    if (code == VALID_INVITE_CODE) {
        return LobbyStatus::Ok;
    }
    if (code == FULL_GAME_INVITE_CODE) {
        return LobbyStatus::GameFull;
    }
    return code == OWN_GAME_INVITE_CODE ? LobbyStatus::AlreadyMember :
                                          LobbyStatus::InvalidInviteCode;
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
        return {LobbyStatus::InvalidName, 0};
    }
    if (request.inviteCode != VALID_INVITE_CODE) {
        return {LobbyStatus::InvalidInviteCode, 0};
    }
    return {LobbyStatus::Ok, INVITE_GAME_ID};
}

LoadGameResponse MockLobbyProtocol::loadGame(uint32_t gameId) {
    bool isMember = std::any_of(savedGames.begin(), savedGames.end(),
                                [&](const SavedGameInfo& game) { return game.gameId == gameId; });
    return {isMember ? LobbyStatus::Ok : LobbyStatus::GameNotFound};
}
