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
};

// La celda vecina a `pos` en esa dirección. Con NONE devuelve `pos`.
Position step(const Position& pos, Direction dir);

#endif  // DIRECTION_H
