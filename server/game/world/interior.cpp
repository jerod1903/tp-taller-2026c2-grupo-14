#include "server/game/world/interior.h"

#include <utility>

#include "server/game/world/point.h"

namespace {
constexpr float EXIT_DISTANCE = 1.5f;
}  // namespace

Interior::Interior(std::string name, Grid grid, const Position& door_pos):
        Location(std::move(name), std::move(grid)), door_pos(door_pos) {}

const Position& Interior::get_door_pos() const { return door_pos; }

void Interior::link_to(Location& outside, const DoorCells& building_door) {
    if (building_door.empty()) {
        return;
    }
    for (const Position& cell: building_door) {
        outside.add_portal(Portal{cell, this, center_of(door_pos), PortalTrigger::Action});
    }
    // El centro de la puerta: entre la primera celda y la última
    Point first = center_of(building_door.front());
    Point last = center_of(building_door.back());
    add_portal(Portal{door_pos, &outside, Point{(first.x + last.x) / 2, first.y + EXIT_DISTANCE},
                      PortalTrigger::Action});
}

void Interior::update(float dt) { update_entities(dt); }
