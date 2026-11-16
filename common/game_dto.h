#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstdint>
#include <string>
#include <vector>

#include "common/lobby_dto.h"

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
    PlayerInfo,
};

// El mundo de la partida, cuántos Snapshots por segundo manda el servidor y el máximo de jugadores
// (el ala este de la cabaña tiene una cama por jugador). Se manda al entrar.
struct LocationData {
    std::string worldId;
    uint16_t tickRate;
    uint8_t maxPlayers;
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

enum class EntityType : uint8_t {
    Player,
    Cow,
    Chicken,
};

// Algo que se dibuja y se mueve, en un tick. entityId es único dentro de su tipo (una vaca puede
// tener el mismo id que un jugador); si es un Player, es su playerId. Posición y dirección como en
// PlayerState.
struct EntityState {
    uint32_t entityId;
    EntityType type;
    std::string areaId;
    uint32_t x;
    uint32_t y;
    int8_t dirX;
    int8_t dirY;

    bool operator==(const EntityState&) const = default;
};

// Estado de la partida en un tick: todos los jugadores conectados y todo lo que se mueve, en
// cualquier área.
struct Snapshot {
    uint32_t tick;
    std::vector<PlayerState> players;
    std::vector<EntityState> entities;
};

// Lo que no cambia de un jugador mientras está en la partida. Al entrar recibe el de cada jugador
// conectado, incluido él, y los demás reciben el suyo. Al salir no se avisa: deja de aparecer en
// los snapshots.
struct PlayerInfo {
    uint32_t playerId;
    std::string characterName;
    CharacterAppearance appearance;

    bool operator==(const PlayerInfo&) const = default;
};

enum class EndReason : uint8_t {
    Left,  // el jugador pidió LeaveGame: la conexión vuelve al lobby
};

// Último mensaje de la partida: lo que llegue después es del lobby.
struct GameEnded {
    EndReason reason;
};

#endif  // GAME_DTO_H
