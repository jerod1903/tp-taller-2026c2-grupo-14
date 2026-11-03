#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstdint>
#include <string>
#include <vector>

// Mensajes del servidor durante la partida: cada uno va precedido por su tag. Lo que manda el
// cliente es un Request (common/requests.h).
enum class GameMsg : uint8_t {
    LocationData,
    Snapshot,
    GameEnded,
};

// El área en la que está el jugador. Se manda al entrar y cada vez que cambia.
struct LocationData {
    std::string worldId;
};

// En milésimas de celda: progress = PROGRESS_SCALE es haber llegado a la celda vecina.
constexpr uint16_t PROGRESS_SCALE = 1000;

// Un jugador en un tick. Está en la celda (x, y) del área areaId; si camina, avanzó `progress`
// hacia la celda vecina en (dirX, dirY), que van en -1, 0 o 1 como en un Request.
struct PlayerState {
    uint32_t playerId;
    std::string areaId;
    uint16_t x;
    uint16_t y;
    int8_t dirX;
    int8_t dirY;
    uint16_t progress;
    uint16_t health;
    uint16_t energy;
    uint32_t gold;

    bool operator==(const PlayerState&) const = default;
};

// Estado de la partida en un tick: todos los jugadores conectados, en cualquier área.
struct Snapshot {
    uint32_t tick;
    std::vector<PlayerState> players;
};

enum class EndReason : uint8_t {
    Left,  // el jugador pidió LeaveGame: la conexión vuelve al lobby
};

// Último mensaje de la partida: lo que llegue después es del lobby.
struct GameEnded {
    EndReason reason;
};

#endif  // GAME_DTO_H
