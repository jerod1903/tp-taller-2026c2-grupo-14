#include "server/game/world/direction.h"

Position step(const Position& pos, Direction dir) {
    switch (dir) {
        case Direction::UP:
            return {pos.x, pos.y - 1};
        case Direction::DOWN:
            return {pos.x, pos.y + 1};
        case Direction::LEFT:
            return {pos.x - 1, pos.y};
        case Direction::RIGHT:
            return {pos.x + 1, pos.y};
        case Direction::NONE:
        default:
            return pos;
    }
}
