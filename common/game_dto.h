#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstdint>
#include <string>
#include <vector>

// Lo que manda el cliente durante la partida: el GameOp y después su payload, si tiene.
enum class GameOp : uint8_t {
    Move = 0x01,
    LeaveGame = 0x02,  // sin payload
};

// -1, 0 o 1 en cada eje; (0, 0) frena.
struct MoveRequest {
    int8_t dirX;
    int8_t dirY;

    bool operator==(const MoveRequest&) const = default;
};

// Mensajes del servidor durante la partida: cada uno va precedido por su tag.
enum class GameMsg : uint8_t {
    LocationData,
    Snapshot,
    GameEnded,
};

// El área en la que está el jugador. Se manda al entrar y cada vez que cambia.
struct LocationData {
    std::string worldId;
};

// Las posiciones van en milésimas de celda: x = 14500 es el centro de la columna 14.
constexpr uint32_t POSITION_SCALE = 1000;

// Caja de colisión de un personaje, centrada en (x, y), en celdas. Tiene el ancho del sprite (10 de
// sus 16 píxeles) y el cliente dibuja los pies en su borde de abajo, así lo que se ve choca justo
// con las paredes. Son exactas en float: quedar pegado a una pared no la hace pisar por redondeo.
constexpr float CHARACTER_HALF_WIDTH = 0.3125f;
constexpr float CHARACTER_HALF_HEIGHT = 0.25f;

// Un jugador en un tick. (x, y) es su centro dentro del área areaId; (dirX, dirY) es hacia dónde
// camina, en -1, 0 o 1 como en un MoveRequest: (0, 0) si está quieto.
struct PlayerState {
    uint32_t playerId;
    std::string areaId;
    uint32_t x;
    uint32_t y;
    int8_t dirX;
    int8_t dirY;
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
