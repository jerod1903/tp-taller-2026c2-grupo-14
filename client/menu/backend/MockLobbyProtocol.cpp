#include "client/menu/backend/MockLobbyProtocol.h"

#include <algorithm>

namespace {
constexpr std::size_t MAX_NAME_LENGTH = 20;
constexpr uint32_t MOCK_PLAYER_ID = 1;
const char* const VALID_INVITE_CODE = "ABC123";
const char* const FULL_GAME_INVITE_CODE = "LLENA1";

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
                {2, "Partida con los chicos", "Pandamos", {7, 5, 22, 3, 4}, true},
                {3, "Prueba de mazmorra", "dipa", {1, 0, 5, 2, 0}, true},
        },
        nextGameId(100) {}

LobbyStatus MockLobbyProtocol::login(const std::string& username) {
    return isValidName(username) ? LobbyStatus::Ok : LobbyStatus::InvalidName;
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
    return LobbyStatus::InvalidInviteCode;
}

GameEnteredResponse MockLobbyProtocol::createGame(const CreateGameRequest& request) {
    if (!isValidName(request.gameName) || !isValidName(request.characterName)) {
        return {LobbyStatus::InvalidName};
    }
    bool worldExists = std::any_of(worlds.begin(), worlds.end(), [&](const WorldInfo& world) {
        return world.worldId == request.worldId;
    });
    if (!worldExists) {
        return {LobbyStatus::WorldNotFound};
    }
    return enterNewGame();
}

GameEnteredResponse MockLobbyProtocol::joinGame(const JoinGameRequest& request) {
    if (!isValidName(request.characterName)) {
        return {LobbyStatus::InvalidName};
    }
    LobbyStatus codeStatus = checkInviteCode(request.inviteCode);
    if (codeStatus != LobbyStatus::Ok) {
        return {codeStatus};
    }
    return enterNewGame();
}

GameEnteredResponse MockLobbyProtocol::loadGame(uint32_t gameId) {
    bool gameExists = std::any_of(savedGames.begin(), savedGames.end(),
                                  [&](const SavedGameInfo& game) { return game.gameId == gameId; });
    if (!gameExists) {
        return {LobbyStatus::GameNotFound};
    }
    return {LobbyStatus::Ok, gameId, MOCK_PLAYER_ID};
}

GameEnteredResponse MockLobbyProtocol::enterNewGame() {
    return {LobbyStatus::Ok, nextGameId++, MOCK_PLAYER_ID};
}
