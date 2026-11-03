#include "server/game/world/tile.h"

Tile::Tile(TileType type, bool walkable): type(type), walkable(walkable) {}

TileType Tile::get_type() const { return type; }

bool Tile::is_walkable() const { return walkable; }
