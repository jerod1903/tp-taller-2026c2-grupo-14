#include "server/game/world/farm/cabin.h"

#include <algorithm>
#include <map>
#include <stdexcept>
#include <utility>
#include <vector>

#include "common/map/CabinInterior.h"

Cabin::Cabin(std::string name, Grid grid, const Position& door_pos,
             std::vector<Position> bed_slots):
        Interior(std::move(name), std::move(grid), door_pos), bed_slots(std::move(bed_slots)) {}

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

SpawnPoint Cabin::bed_spawn(uint32_t player_id) {
    const Position& bed = assign_bed(player_id);
    constexpr float CELL_CENTER = 0.5f;
    return SpawnPoint{*this, Point{bed.x + stardew::maps::BED_WIDTH / 2.0f,
                                   bed.y + stardew::maps::BED_HEAD_HEIGHT + CELL_CENTER}};
}

const std::map<uint32_t, Position>& Cabin::get_beds() const { return beds; }

std::size_t Cabin::bed_slot_count() const { return bed_slots.size(); }

bool Cabin::is_walkable(const Position& pos) const {
    return Location::is_walkable(pos) && !is_bed_head(pos);
}

bool Cabin::is_bed_head(const Position& pos) const {
    return std::any_of(beds.begin(), beds.end(), [&pos](const auto& bed) {
        const Position& corner = bed.second;
        // Solo la fila de la almohada: por la de arriba se pasa por detras de la cabecera
        return pos.x >= corner.x && pos.x < corner.x + stardew::maps::BED_WIDTH &&
               pos.y == corner.y + stardew::maps::BED_HEAD_HEIGHT - 1;
    });
}
