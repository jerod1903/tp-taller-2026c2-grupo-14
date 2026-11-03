#include "server/game/world/dungeon/dungeon_level.h"

#include <utility>

DungeonLevel::DungeonLevel(std::string name, Grid grid, int depth, const Position& stairs_down):
        Location(std::move(name), std::move(grid)),
        depth(depth),
        exit_portal(),
        stairs_down(stairs_down) {}

int DungeonLevel::get_depth() const { return depth; }

const Position& DungeonLevel::get_stairs_down() const { return stairs_down; }

void DungeonLevel::set_exit_portal(const Portal& portal) {
    exit_portal = portal;
    add_portal(portal);
}

const Portal& DungeonLevel::get_exit_portal() const { return exit_portal; }

void DungeonLevel::update(float dt) { update_entities(dt); }
