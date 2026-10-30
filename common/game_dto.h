#ifndef GAME_DTO_H
#define GAME_DTO_H

#include <cstdint>
#include <variant>

enum class GameOp : uint8_t {
    Move = 0,   // cliente -> servidor
    State = 1,  // servidor -> cliente
};

enum class Direction : uint8_t { Up, Down, Left, Right };

struct MoveCommand {
    Direction direction;
};

struct PlayerState {
    uint32_t id;
    uint32_t x;
    uint32_t y;
    uint32_t life;
    uint32_t energy;
    uint32_t coins;
};

using ClientMessage = std::variant<MoveCommand, PicarCommand>;
using ServerMessage = std::variant<PlayerState>;

#endif  // GAME_DTO_H