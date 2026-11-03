#ifndef WORLD_HELPERS_H
#define WORLD_HELPERS_H

#include <cstdint>
#include <utility>
#include <vector>

#include "server/game/characters/creature.h"
#include "server/game/world/grid.h"
#include "server/game/world/location.h"

// Grilla de pasto caminable, con paredes en `walls`.
inline Grid make_grid(int width = 5, int height = 5, const std::vector<Position>& walls = {}) {
    std::vector<Tile> tiles(width * height, Tile(TileType::Grass, true));
    for (const Position& wall: walls) {
        tiles[wall.y * width + wall.x] = Tile(TileType::CaveWall, false);
    }
    return Grid(width, height, std::move(tiles));
}

// Location concreta mínima: solo actualiza sus entidades.
class TestLocation: public Location {
public:
    explicit TestLocation(Grid grid = make_grid()): Location("test", std::move(grid)) {}
    void update(float dt) override { update_entities(dt); }
};

// Creature que solo camina y cuenta sus updates.
class TestCreature: public Creature {
public:
    TestCreature(Location& location, const Position& pos = {0, 0}, float speed = 1.0f):
            Creature(next_id++, pos, location, 10, speed) {}
    void update(float dt) override {
        ++updates;
        advance(dt);
    }
    int updates = 0;

private:
    static inline uint32_t next_id = 1;
};

#endif  // WORLD_HELPERS_H
