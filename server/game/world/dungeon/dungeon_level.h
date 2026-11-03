#ifndef DUNGEON_LEVEL_H
#define DUNGEON_LEVEL_H

#include <string>

#include "server/game/world/location.h"
#include "server/game/world/portal.h"
#include "server/game/world/position.h"

class DungeonLevel: public Location {
public:
    DungeonLevel(std::string name, Grid grid, int depth, const Position& stairs_down);

    int get_depth() const;
    const Position& get_stairs_down() const;

    // La salida al pueblo. Se fija al enlazar el mundo, cuando el Town ya existe.
    void set_exit_portal(const Portal& portal);
    const Portal& get_exit_portal() const;

    // TODO: enemigos (el nivel es dueño de ellos y los registra como entidades de la Location).
    void update(float dt) override;

private:
    int depth;
    Portal exit_portal;
    Position stairs_down;
};

#endif  // DUNGEON_LEVEL_H
