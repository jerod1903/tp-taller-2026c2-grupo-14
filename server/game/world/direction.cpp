#include "server/game/world/direction.h"

#include <cmath>

namespace {
// Menos que esto en un eje cuenta como alineado
constexpr float ALIGNED = 0.1f;

// -1, 0 o 1 según hacia qué lado hay que ir en ese eje
int sign_of(float delta) {
    if (std::fabs(delta) < ALIGNED) {
        return 0;
    }
    return delta > 0 ? 1 : -1;
}
}  // namespace

Direction direction_towards(float dx, float dy) {
    int horizontal = sign_of(dx);
    int vertical = sign_of(dy);
    // En cada caso: si hay que ir a la izquierda, a la derecha o en ninguna de las dos
    if (vertical < 0) {
        return horizontal < 0 ? Direction::UP_LEFT :
                                (horizontal > 0 ? Direction::UP_RIGHT : Direction::UP);
    }
    if (vertical > 0) {
        return horizontal < 0 ? Direction::DOWN_LEFT :
                                (horizontal > 0 ? Direction::DOWN_RIGHT : Direction::DOWN);
    }
    return horizontal < 0 ? Direction::LEFT : (horizontal > 0 ? Direction::RIGHT : Direction::NONE);
}

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
        case Direction::UP_LEFT:
            return {pos.x - 1, pos.y - 1};
        case Direction::UP_RIGHT:
            return {pos.x + 1, pos.y - 1};
        case Direction::DOWN_LEFT:
            return {pos.x - 1, pos.y + 1};
        case Direction::DOWN_RIGHT:
            return {pos.x + 1, pos.y + 1};
        case Direction::NONE:
        default:
            return pos;
    }
}
