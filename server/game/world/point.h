#ifndef POINT_H
#define POINT_H

#include <cmath>

#include "server/game/world/position.h"

// Un punto de una Location, en celdas y con decimales. Position es la celda entera que lo contiene.
struct Point {
    float x;
    float y;
};

inline Position cell_of(const Point& point) {
    return Position{static_cast<int>(std::floor(point.x)), static_cast<int>(std::floor(point.y))};
}

#endif  // POINT_H
