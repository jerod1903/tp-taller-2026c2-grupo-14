#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <array>
#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

constexpr uint8_t PROTOCOL_VERSION = 1;

constexpr std::size_t USER_ID_SIZE = 16;
using UserId = std::array<uint8_t, USER_ID_SIZE>;

enum class LobbyOp : uint8_t {
    Login,
    ListWorlds,
    ListMyGames,
    CheckInviteCode,
    JoinGame,
    CreateGame,
    LoadGame,
};

enum class LobbyStatus : uint8_t {
    Ok,
    InvalidName,  // usuario, partida o personaje vacío, muy largo o con caracteres no permitidos
    InvalidAppearance,  // algún id no existe en el catálogo
    WorldNotFound,
    GameNotFound,       // no existe, o el usuario no participa (misma respuesta en ambos casos)
    InvalidInviteCode,  // código inexistente o partida single (misma respuesta en ambos casos)
    GameFull,           // se alcanzó el máximo de jugadores (config TOML)
    NameTaken,          // el nombre de personaje ya existe en esa partida
    AlreadyConnected,   // ese userId ya está conectado a la partida
    GameClosed,         // la partida se está cerrando
    ProtocolMismatch,   // versión de protocolo incompatible
    AlreadyMember,      // JOIN_GAME de alguien que ya está en la partida: tiene que usar LOAD_GAME
};

constexpr std::string_view to_string(LobbyStatus status) {
    switch (status) {
        case LobbyStatus::Ok:
            return "Ok";
        case LobbyStatus::InvalidName:
            return "InvalidName";
        case LobbyStatus::InvalidAppearance:
            return "InvalidAppearance";
        case LobbyStatus::WorldNotFound:
            return "WorldNotFound";
        case LobbyStatus::GameNotFound:
            return "GameNotFound";
        case LobbyStatus::InvalidInviteCode:
            return "InvalidInviteCode";
        case LobbyStatus::GameFull:
            return "GameFull";
        case LobbyStatus::NameTaken:
            return "NameTaken";
        case LobbyStatus::AlreadyConnected:
            return "AlreadyConnected";
        case LobbyStatus::GameClosed:
            return "GameClosed";
        case LobbyStatus::ProtocolMismatch:
            return "ProtocolMismatch";
        case LobbyStatus::AlreadyMember:
            return "AlreadyMember";
    }
    return "Unknown";
}

struct LoginRequest {
    uint8_t protocolVersion;
    UserId userId;         // UUID v4 generado y guardado por el cliente
    std::string username;  // solo para mostrar
};

// Ids = índices del catálogo de apariencia.
struct CharacterAppearance {
    uint16_t hairStyle;
    uint8_t hairColor;
    uint16_t shirt;
    uint16_t pants;
    uint16_t hat;  // 0 = sin sombrero
};

struct WorldInfo {
    std::string worldId;  // se valida contra la lista del servidor, nunca se usa para armar rutas
    std::string name;
};

struct SavedGameInfo {
    uint32_t gameId;
    std::string gameName;
    std::string worldName;
    std::string characterName;
    CharacterAppearance appearance;
    bool isMultiplayer;
};

struct WorldListResponse {
    std::vector<WorldInfo> worlds;
};

struct SavedGameListResponse {
    std::vector<SavedGameInfo> games;
};

struct CheckInviteCodeRequest {
    std::string inviteCode;
};

struct CheckInviteCodeResponse {
    LobbyStatus status;  // Ok, InvalidInviteCode, GameFull
    bool alreadyMember;  // válido solo si status == Ok
    uint32_t gameId;     // válido solo si alreadyMember: con él se hace LOAD_GAME
};

// Entrar por primera vez a una partida multi. Va con el código y no con el gameId: los ids son
// secuenciales y cualquiera podría adivinarlos, el código es lo que autoriza a entrar.
struct JoinGameRequest {
    std::string inviteCode;
    std::string characterName;
    CharacterAppearance appearance;
};

struct JoinGameResponse {
    // Ok, InvalidName, InvalidAppearance, InvalidInviteCode, GameFull, NameTaken, AlreadyMember,
    // GameClosed
    LobbyStatus status;
    uint32_t gameId;  // válido solo si status == Ok
};

// Toda partida arranca single: no se pide si es multi.
struct CreateGameRequest {
    std::string worldId;
    std::string gameName;
    std::string characterName;
    CharacterAppearance appearance;
};

struct CreateGameResponse {
    LobbyStatus status;  // Ok, InvalidName, InvalidAppearance, WorldNotFound
    uint32_t gameId;     // válido solo si status == Ok
};

// Retomar una partida de la que ya se es miembro: el personaje es el guardado.
struct LoadGameRequest {
    uint32_t gameId;
};

struct LoadGameResponse {
    LobbyStatus status;  // Ok, GameNotFound, AlreadyConnected, GameClosed
};

#endif  // LOBBY_DTO_H
