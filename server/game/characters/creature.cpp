#include "server/game/characters/creature.h"

#include <algorithm>

#include "server/game/world/location.h"

Creature::Creature(uint32_t id, const Position& pos, Location& location, int health,
                   float base_speed):
        Entity(id, pos, location), health(health), base_speed(base_speed) {}

int Creature::get_health() const { return health; }

void Creature::take_damage(int amount) { health = std::max(0, health - amount); }

bool Creature::is_alive() const { return health > 0; }

float Creature::speed() const { return base_speed; }

void Creature::set_direction(Direction dir) {
    if (dir != direction) {
        direction = dir;
        progress = 0.0f;
    }
}

Direction Creature::get_direction() const { return direction; }

float Creature::get_progress() const { return progress; }

bool Creature::has_stepped() const { return stepped; }

bool Creature::advance(float dt) {
    stepped = false;
    if (direction == Direction::NONE) {
        return false;
    }
    progress += speed() * dt;
    if (progress < 1.0f) {
        return false;
    }
    Position next = step(pos, direction);
    if (!location->is_walkable(next)) {
        progress = 0.0f;  // Contra una pared o el borde: se queda quieto.
        return false;
    }
    pos = next;
    progress -= 1.0f;
    stepped = true;
    return true;
}
