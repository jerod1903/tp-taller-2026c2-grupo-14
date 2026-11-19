#include "server/game/world/dungeon/traps/flamethrower.h"

#include <vector>

#include "server/game/world/location.h"

Flamethrower::Flamethrower(uint32_t id, const Position& pos, Location& location,
                           const std::vector<Creature*>& targets, Direction facing, float start):
        Trap(id, pos, location, targets, server_config().traps.flamethrower.trap, start),
        facing(facing),
        reach(server_config().traps.flamethrower.reach) {}

EntityType Flamethrower::entity_type() const {
    switch (facing) {
        case Direction::LEFT:
            return EntityType::FlamethrowerLeft;
        case Direction::RIGHT:
            return EntityType::FlamethrowerRight;
        default:
            return EntityType::FlamethrowerDown;
    }
}

bool Flamethrower::covers(const Position& cell) const {
    Position burning = pos;
    for (int i = 0; i < reach && location->is_walkable(burning); ++i) {
        if (burning == cell) {
            return true;
        }
        burning = step(burning, facing);
    }
    return false;
}
