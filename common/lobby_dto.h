#ifndef LOBBY_DTO_H
#define LOBBY_DTO_H

#include <array>
#include <cstdint>
#include <string>
#include <string_view>

constexpr uint8_t PROTOCOL_VERSION = 1;

constexpr std::size_t USER_ID_SIZE = 16;
using UserId = std::array<uint8_t, USER_ID_SIZE>;

enum class LobbyOp : uint8_t {
    Login,
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
    }
    return "Unknown";
}

struct LoginRequest {
    uint8_t protocolVersion;
    UserId userId;         // UUID v4 generado y guardado por el cliente
    std::string username;  // solo para mostrar
};

struct CheckInviteCodeResponse {
    LobbyStatus status;  // Ok, InvalidInviteCode, GameFull
    bool alreadyMember;  // válido solo si status == Ok
    uint32_t gameId;     // válido solo si alreadyMember
};

#endif  // LOBBY_DTO_H
