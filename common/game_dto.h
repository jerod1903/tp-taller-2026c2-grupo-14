#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstddef>
#include <cstdint>
#include <string>
#include <vector>

#include "common/lobby_dto.h"
#include "common/farm_dto.h"

// Lo que manda el cliente durante la partida: el GameOp y después su payload, si tiene.
enum class GameOp : uint8_t {
    Move = 0x01,
    LeaveGame = 0x02,  // sin payload
    SelectSlot = 0x03,
    Chat = 0x04,         // payload: string con el texto tal cual lo escribio el jugador
    Interact = 0x05,     // sin payload: el servidor decide con que interactua (por ahora, ordeñar)
    Typing = 0x06,       // payload: u8, 1 si abrio el chat y 0 si lo cerro
    EnterPortal = 0x07,  // sin payload: cruza la puerta que el jugador tiene al alcance
    UseTool = 0x08,      // sin payload: usa lo del slot elegido sobre la celda que tiene adelante
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
    InventoryState,
    ChatMessage,
    FarmState,
};

// El mundo de la partida, cuántos Snapshots por segundo manda el servidor, el máximo de jugadores
// (el ala este de la cabaña tiene una cama por jugador) y la energía máxima de un jugador (para la
// barra del HUD). Se manda al entrar.
struct LocationData {
    std::string worldId;
    uint16_t tickRate;
    uint8_t maxPlayers;
    uint16_t maxEnergy;
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
    Bed,  // la de cada jugador, con su playerId como entityId; aparece cuando entra a la partida
};

// Lo que se muestra arriba de la cabeza de una entidad.
enum class Emote : uint8_t {
    None,
    CowMilkReady,  // una vaca lista para ordeñar
    Typing,        // un jugador con el chat abierto
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
    Emote emote = Emote::None;  // en el cable va al final, como u8

    bool operator==(const EntityState&) const = default;
};

// Estado de la partida en un tick: todos los jugadores conectados y todo lo que se mueve, en
// cualquier área.
struct Snapshot {
    uint32_t tick;
    uint16_t day;          // día 1, 2, ...
    uint16_t minuteOfDay;  // minutos de juego desde las 6:00 (0..1199)
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

// Slot del inventario que el jugador elige: 0 a INVENTORY_SIZE - 1. El servidor valida el rango.
struct SelectSlotRequest {
    uint8_t slot;

    bool operator==(const SelectSlotRequest&) const = default;
};


// Si el jugador tiene el chat abierto. El cliente lo manda solo cuando cambia; los demas le ven
// el globito de "escribiendo" (Emote::Typing).
struct TypingRequest {
    bool typing;

    bool operator==(const TypingRequest&) const = default;
};

// Largo maximo de un mensaje de chat, en bytes. El servidor corta lo que sobra.
constexpr std::size_t MAX_CHAT_LENGTH = 80;

// Lo que escribio el jugador en el chat. El servidor decide a quien le llega: "@nombre texto" es
// privado para ese personaje; cualquier otro texto es publico.
struct ChatRequest {
    std::string text;

    bool operator==(const ChatRequest&) const = default;
};

enum class ChatKind : uint8_t {
    Public,   // a todos los jugadores de la partida
    Private,  // solo al destinatario, con una copia para el que lo mando
    System,   // un aviso del servidor (por ejemplo, que el destinatario no esta conectado)
};

// Un mensaje de chat. En el cable: tag GameMsg::ChatMessage, u8 kind, from y text como strings.
// En System, from va vacio.
struct ChatMessage {
    ChatKind kind;
    std::string from;  // nombre del personaje que lo escribio
    std::string text;

    bool operator==(const ChatMessage&) const = default;
};

// Cantidad de slots del inventario de un jugador.
constexpr uint8_t INVENTORY_SIZE = 9;

enum class ItemType : uint8_t {
    Empty = 0,
    Pickaxe,
    WateringCan,
    Shovel,
    Sword,
    Milk,
    StrawberrySeeds,
    ScallionSeeds,
    PotatoSeeds,
    PumpkinSeeds,
    Strawberry,
    Scallion,
    Potato,
    Pumpkin,
    Egg,
};

struct InventorySlot {
    ItemType item;
    uint8_t quantity;
    ItemQuality quality = ItemQuality::Base;

    bool operator==(const InventorySlot&) const = default;
};


// Inventario de un jugador. Se manda al entrar y cada vez que cambia, no en cada tick.
// En el cable: tag GameMsg::InventoryState, u8 selectedSlot, u16 cantidad de slots y, por cada
// slot, u8 item, u8 quantity y u8 quality. Con selectedSlot de 0 a slots.size() - 1.
struct InventoryState {
    uint8_t selectedSlot = 0;
    std::vector<InventorySlot> slots = {};

    bool operator==(const InventoryState&) const = default;
};


#endif  // GAME_DTO_H
