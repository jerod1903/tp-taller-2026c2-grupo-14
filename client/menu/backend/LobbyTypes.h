#pragma once

#include <cstdint>
#include <string>
#include <vector>

// Provisorio: copia de la propuesta de DTOs del lobby. Cuando existan en common/,
// este archivo se borra y se incluyen los de common.

enum class LobbyStatus : uint8_t {
    Ok,
    InvalidName,
    InvalidAppearance,
    WorldNotFound,
    GameNotFound,
    InvalidInviteCode,
    GameFull,
};

// Ids = indices del catalogo de apariencia.
struct CharacterAppearance {
    uint16_t hairStyle = 0;
    uint8_t hairColor = 0;
    uint16_t shirt = 0;
    uint16_t pants = 0;
    uint16_t hat = 0;  // 0 = sin sombrero
};

struct WorldInfo {
    std::string worldId;
    std::string name;
};

struct SavedGameInfo {
    uint32_t gameId = 0;
    std::string gameName;
    std::string worldName;
    std::string characterName;
    CharacterAppearance appearance;
    bool isMultiplayer = false;
};

struct CreateGameRequest {
    std::string gameName;
    std::string worldId;
    std::string characterName;
    CharacterAppearance appearance;
};

struct JoinGameRequest {
    std::string inviteCode;
    std::string characterName;
    CharacterAppearance appearance;
};

// Respuesta comun a crear, unirse y retomar. gameId y playerId valen solo si status == Ok.
struct GameEnteredResponse {
    LobbyStatus status = LobbyStatus::Ok;
    uint32_t gameId = 0;
    uint32_t playerId = 0;
};
