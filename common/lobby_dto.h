#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <cstdint>
#include <string>
#include <string_view>
#include <vector>

constexpr uint8_t PROTOCOL_VERSION = 1;

enum class LobbyOp : uint8_t {
    Login,
    ListWorlds,
    ListMyGames,
    CheckInviteCode,
    JoinGame,
    CreateGame,
    LoadGame,
    EnableMultiplayer,
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
    AlreadyConnected,   // ese usuario ya está conectado a la partida
    GameClosed,         // la partida se está cerrando
    ProtocolMismatch,   // versión de protocolo incompatible
    AlreadyMember,      // ya está en la partida del código: tiene que entrar con LOAD_GAME
    NotOwner,           // la operación es solo para el creador de la partida
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
        case LobbyStatus::NotOwner:
            return "NotOwner";
    }
    return "Unknown";
}

struct LoginRequest {
    uint8_t protocolVersion;
    std::string username;  // identifica al jugador: con él se recuperan sus partidas
};

// Ids = índices del catálogo de apariencia.
struct CharacterAppearance {
    uint16_t hairStyle;
    uint8_t hairColor;
    uint16_t shirt;
    uint16_t pants;
    uint16_t hat;  // 0 = sin sombrero

    bool operator==(const CharacterAppearance&) const = default;
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

// Responde solo un status: Ok, InvalidInviteCode, GameFull, AlreadyMember.
struct CheckInviteCodeRequest {
    std::string inviteCode;
};

// Entrar por primera vez a una partida multi. Va con el código y no con el gameId: los ids son
// secuenciales y cualquiera podría adivinarlos, el código es lo que autoriza a entrar.
struct JoinGameRequest {
    std::string inviteCode;
    std::string characterName;
    CharacterAppearance appearance;
};

// Toda partida arranca single: no se pide si es multi.
struct CreateGameRequest {
    std::string worldId;
    std::string gameName;
    std::string characterName;
    CharacterAppearance appearance;
};

// Retomar una partida de la que ya se es miembro: el personaje es el guardado.
struct LoadGameRequest {
    uint32_t gameId;
};

// Respuesta común a CREATE_GAME, JOIN_GAME y LOAD_GAME. Status posibles:
// - CREATE_GAME: Ok, InvalidName, InvalidAppearance, WorldNotFound
// - JOIN_GAME: Ok, InvalidName, InvalidAppearance, InvalidInviteCode, GameFull, NameTaken,
//   AlreadyMember, WorldNotFound (el mapa de la partida ya no se carga)
// - LOAD_GAME: Ok, GameNotFound, AlreadyConnected, WorldNotFound
struct GameEnteredResponse {
    LobbyStatus status;
    uint32_t gameId;    // válido solo si status == Ok
    uint32_t playerId;  // válido solo si status == Ok; identifica al jugador en los snapshots
};

// El creador pasa a multi su partida y recibe el código de invitación. Si ya era multi devuelve el
// mismo código: sirve también para volver a consultarlo.
struct EnableMultiplayerRequest {
    uint32_t gameId;
};

struct EnableMultiplayerResponse {
    LobbyStatus status;      // Ok, GameNotFound, NotOwner
    std::string inviteCode;  // vacío si status != Ok
};

#endif  // LOBBY_DTO_H
