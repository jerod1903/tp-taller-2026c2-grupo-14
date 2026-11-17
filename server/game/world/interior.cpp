#include "server/game/world/interior.h"

#include <utility>

#include "server/game/world/point.h"

namespace {
constexpr float EXIT_DISTANCE = 1.5f;
}  // namespace

Interior::Interior(std::string name, Grid grid, const Position& door_pos):
        Location(std::move(name), std::move(grid)), door_pos(door_pos) {}

const Position& Interior::get_door_pos() const { return door_pos; }

void Interior::link_to(Location& outside, const Position& building_door) {
    outside.add_portal(Portal{building_door, this, center_of(door_pos), PortalTrigger::Action});
    Point door = center_of(building_door);
    add_portal(Portal{door_pos, &outside, Point{door.x, door.y + EXIT_DISTANCE},
                      PortalTrigger::Action});
}

void Interior::update(float dt) { update_entities(dt); }
