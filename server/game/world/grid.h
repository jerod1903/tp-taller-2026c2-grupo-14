#ifndef GRID_H
#define GRID_H

#include <vector>

#include "server/game/world/position.h"
#include "server/game/world/tile.h"

// Matriz width x height de tiles. Se guarda por filas: el tile (x, y) está en y * width + x.
class Grid {
public:
    Grid(int width, int height, std::vector<Tile> tiles);

    int get_width() const;
    int get_height() const;

    bool in_bounds(const Position& pos) const;

    const Tile& at(const Position& pos) const;

private:
    int width;
    int height;
    std::vector<Tile> tiles;
};

#endif  // GRID_H
