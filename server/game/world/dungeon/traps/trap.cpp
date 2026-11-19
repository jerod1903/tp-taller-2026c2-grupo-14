#include "server/game/world/dungeon/traps/trap.h"

#include <cmath>
#include <vector>

Trap::Trap(uint32_t id, const Position& pos, Location& location,
           const std::vector<Creature*>& targets, const ServerConfig::Trap& stats, float start):
        Entity(id, pos, location),
        targets(targets),
        damage(stats.damage),
        active_time(stats.active_time),
        cycle(stats.active_time + stats.idle_time),
        time(std::fmod(start, cycle)) {}

bool Trap::is_active() const { return time < active_time; }

void Trap::update(float dt) {
    time += dt;
    if (time >= cycle) {
        // Empieza otra activación
        time = std::fmod(time, cycle);
        hurt.clear();
    }
    if (is_active()) {
        hurt_the_ones_inside();
    }
}

void Trap::hurt_the_ones_inside() {
    for (Creature* target: targets) {
        if (target->is_alive() && &target->get_location() == location &&
            covers(target->get_position()) && !hurt.contains(target)) {
            target->take_damage(damage);
            hurt.insert(target);
        }
    }
}
