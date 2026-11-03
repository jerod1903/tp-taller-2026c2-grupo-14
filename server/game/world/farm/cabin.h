#ifndef CABIN_H
#define CABIN_H

#include <string>

#include "server/game/world/location.h"
#include "server/game/world/position.h"

// El interior de la cabaña. Es una Location propia, pero vive dentro de la Farm.
class Cabin: public Location {
public:
    Cabin(std::string name, Grid grid, const Position& door_pos);

    // Tile interior que lleva de vuelta a la granja.
    const Position& get_door_pos() const;

    void update(float dt) override;

private:
    Position door_pos;
};

#endif  // CABIN_H
