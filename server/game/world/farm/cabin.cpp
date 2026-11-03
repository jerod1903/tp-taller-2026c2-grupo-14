#include "server/game/world/farm/cabin.h"

#include <utility>

Cabin::Cabin(std::string name, Grid grid, const Position& door_pos):
        Location(std::move(name), std::move(grid)), door_pos(door_pos) {}

const Position& Cabin::get_door_pos() const { return door_pos; }

void Cabin::update(float dt) { update_entities(dt); }
