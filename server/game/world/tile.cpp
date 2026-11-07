#include "server/game/world/tile.h"

bool is_walkable_terrain(TileType type) { return stardew::maps::isWalkableTerrain(type); }

Tile::Tile(TileType type, bool walkable): type(type), walkable(walkable) {}

TileType Tile::get_type() const { return type; }

bool Tile::is_walkable() const { return walkable; }
