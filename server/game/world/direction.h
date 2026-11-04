#ifndef DIRECTION_H
#define DIRECTION_H

#include <cstdint>

#include "server/game/world/position.h"

enum class Direction : uint8_t {
    NONE,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    UP_LEFT,
    UP_RIGHT,
    DOWN_LEFT,
    DOWN_RIGHT,
};

// La celda vecina a `pos` en esa dirección (en diagonal, la de la esquina). Con NONE devuelve
// `pos`.
Position step(const Position& pos, Direction dir);

#endif  // DIRECTION_H
