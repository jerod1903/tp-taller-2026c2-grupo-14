#ifndef POSITION_H
#define POSITION_H

// Coordenada de una celda dentro de la grilla de una Location.
struct Position {
    int x;
    int y;

    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Position& other) const { return !(*this == other); }
};

#endif  // POSITION_H
