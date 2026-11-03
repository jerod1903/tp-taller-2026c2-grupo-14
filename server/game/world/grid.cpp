#include "server/game/world/grid.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

Grid::Grid(int width, int height, std::vector<Tile> tiles):
        width(width), height(height), tiles(std::move(tiles)) {
    if (width < 0 || height < 0 ||
        this->tiles.size() != static_cast<size_t>(width) * static_cast<size_t>(height)) {
        throw std::invalid_argument("la grilla no tiene width * height tiles");
    }
}

int Grid::get_width() const { return width; }

int Grid::get_height() const { return height; }

bool Grid::in_bounds(const Position& pos) const {
    return pos.x >= 0 && pos.x < width && pos.y >= 0 && pos.y < height;
}

const Tile& Grid::at(const Position& pos) const { return tiles.at(pos.y * width + pos.x); }
