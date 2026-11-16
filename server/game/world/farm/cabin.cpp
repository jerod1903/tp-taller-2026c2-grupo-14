#include "server/game/world/farm/cabin.h"

#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

Cabin::Cabin(std::string name, Grid grid, const Position& door_pos,
             std::vector<Position> bed_slots):
        Location(std::move(name), std::move(grid)),
        door_pos(door_pos),
        bed_slots(std::move(bed_slots)) {}

const Position& Cabin::get_door_pos() const { return door_pos; }

const Position& Cabin::assign_bed(uint32_t player_id) {
    auto bed = beds.find(player_id);
    if (bed != beds.end()) {
        return bed->second;
    }
    // Las camas no se liberan, así que la próxima libre es la que sigue a las ya asignadas.
    if (beds.size() >= bed_slots.size()) {
        throw std::logic_error("no quedan lugares para camas en la cabaña");
    }
    return beds.emplace(player_id, bed_slots[beds.size()]).first->second;
}

const std::map<uint32_t, Position>& Cabin::get_beds() const { return beds; }

std::size_t Cabin::bed_slot_count() const { return bed_slots.size(); }

void Cabin::update(float dt) { update_entities(dt); }
