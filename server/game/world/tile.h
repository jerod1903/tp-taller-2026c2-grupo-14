#ifndef TILE_H
#define TILE_H

#include "common/map/MapDefinition.h"

using stardew::maps::TileType;

// Agua, paredes de la caverna y piedra no se caminan; el resto del terreno sí.
bool is_walkable_terrain(TileType type);

// Una celda de la grilla. Si es caminable se decide al construirla: además del terreno, la puede
// tapar una estructura o un NPC del mapa.
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
