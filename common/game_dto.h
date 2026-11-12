#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstdint>
#include <string>
#include <vector>

#include "common/lobby_dto.h"


// PROTOCOLO DE PARTIDA 
// 1. Cliente -> Servidor (Opcode GameOp u8 adelante):
//    - Move (0x01):       i8 dirX, i8 dirY
//    - LeaveGame (0x02):  sin payload (1 byte en total)
//
// 2. Servidor -> Cliente (Tag GameMsg u8 adelante):
//    - LocationData (0x00):
//        string worldId  (u16 length + bytes UTF-8)
//    - Snapshot (0x01):
//        u32 tick
//        u16 players_count
//        por cada jugador:
//           (PlayerState):
//                u32 playerId
//                string areaId
//                u32 x, u32 y
//                i8 dirX, i8 dirY
//                u16 health, u16 energy, u32 gold
//                u16 entities_count
//        por cada entidad 
//           (EntityState):
//              u32 entityId
//              u8 type (EntityType)
//              string areaId
//              u32 x, u32 y
//              i8 dirX, i8 dirY
//    - GameEnded (0x02):
//        u8 reason (EndReason)
//    - PlayerInfo (0x03):
//        u32 playerId
//        string characterName
//        CharacterAppearance (hairStyle u16, hairColor u8, shirt u16, pants u16, hat u16)


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

enum class GameMsg : uint8_t {
    LocationData,
    Snapshot,
    GameEnded,
    PlayerInfo,
};

// Tipo de entidad animada/visible en el mapa.
enum class EntityType : uint8_t {
    Player,
    Cow,
    Chicken,
};

// Estado posicional y de animación de cualquier cosa visible en un tick.
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

// El área en la que está el jugador. Se manda al entrar y cada vez que cambia.
struct LocationData {
    std::string worldId;
};

// Información estática de un jugador. Se manda al entrar y a los que ya estaban.
struct PlayerInfo {
    uint32_t playerId;
    std::string characterName;
    CharacterAppearance appearance;

    bool operator==(const PlayerInfo&) const = default;
};

// Las posiciones van en milésimas de celda: x = 14500 es el centro de la columna 14.
constexpr uint32_t POSITION_SCALE = 1000;

// Caja de colisión de un personaje, centrada en (x, y), en celdas. Tiene el ancho del sprite (10 de
// sus 16 píxeles) y el cliente dibuja los pies en su borde de abajo, así lo que se ve choca justo
// con las paredes. Son exactas en float: quedar pegado a una pared no la hace pisar por redondeo.
constexpr float CHARACTER_HALF_WIDTH = 0.3125f;
constexpr float CHARACTER_HALF_HEIGHT = 0.25f;

// Un jugador en un tick. (x, y) es su centro dentro del área areaId; (dirX, dirY) es hacia dónde
// camina, en -1, 0 o 1 como en un Request: (0, 0) si está quieto.
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

// Estado de la partida en un tick: jugadores y entidades visibles en cualquier área.
struct Snapshot {
    uint32_t tick;
    std::vector<PlayerState> players;
    std::vector<EntityState> entities{};

    bool operator==(const Snapshot&) const = default;
};

enum class EndReason : uint8_t {
    Left,  // el jugador pidió LeaveGame: la conexión vuelve al lobby
};

// Último mensaje de la partida: lo que llegue después es del lobby.
struct GameEnded {
    EndReason reason;
};

#endif  // GAME_DTO_H
