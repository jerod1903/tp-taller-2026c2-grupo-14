#ifndef TILE_H
#define TILE_H

#include <cstdint>

enum class TileType : uint8_t {
    GRASS,
    TILLABLE,
    DIRT,
    WATER,
    WALL,
    STONE,
};

// Una celda de la grilla. Si es caminable se decide al construirla, no se deriva del tipo.
class Tile {
public:
    Tile(TileType type, bool walkable);

    TileType get_type() const;
    bool is_walkable() const;

private:
    TileType type;
    bool walkable;
};

#endif  // TILE_H
