#include "server/game/world/town/town.h"

#include <utility>

Town::Town(std::string name, Grid grid): Location(std::move(name), std::move(grid)) {}

void Town::update(float dt) { update_entities(dt); }
