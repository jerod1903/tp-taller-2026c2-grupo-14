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

// Hacia dónde ir para recorrer el vector (dx, dy): una de las 8 direcciones, o NONE si ya llegó. Un
// eje casi alineado (menos de una décima de celda) no cuenta, así no tiembla al estar alineado.
Direction direction_towards(float dx, float dy);

#endif  // DIRECTION_H
