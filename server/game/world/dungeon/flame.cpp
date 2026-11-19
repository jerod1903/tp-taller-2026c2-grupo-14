#include "server/game/world/dungeon/flame.h"

#include <cmath>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/dungeon_level.h"

namespace {
// Toca a alguien cuando su centro le queda a menos de esto, en celdas
constexpr float HIT_DISTANCE = 0.5f;
// La verde: el doble de daño a la mitad de velocidad
constexpr int GREEN_DAMAGE_FACTOR = 2;
constexpr float GREEN_SPEED_FACTOR = 0.5f;

// De largo 1. Si los dos puntos coinciden, hacia abajo.
Point unit_towards(const Point& from, const Point& towards) {
    float dx = towards.x - from.x;
    float dy = towards.y - from.y;
    float length = std::hypot(dx, dy);
    if (length == 0) {
        return Point{0, 1};
    }
    return Point{dx / length, dy / length};
}
}  // namespace

Flame::Flame(uint32_t id, const Point& from, const Point& towards, DungeonLevel& level,
             const std::vector<Creature*>& targets, FlameKind kind):
        Entity(id, cell_of(from), level),
        level(level),
        targets(targets),
        kind(kind),
        center(from),
        direction(unit_towards(from, towards)),
        speed(server_config().enemies.flame.speed),
        damage(server_config().enemies.flame.damage),
        range_left(server_config().enemies.flame.range) {
    if (kind == FlameKind::Green) {
        speed *= GREEN_SPEED_FACTOR;
        damage *= GREEN_DAMAGE_FACTOR;
    }
    level.add_flame(*this);
}

Flame::~Flame() { level.remove_flame(*this); }

EntityType Flame::entity_type() const {
    return kind == FlameKind::Green ? EntityType::GreenFlame : EntityType::Flame;
}

float Flame::get_x() const { return center.x; }

float Flame::get_y() const { return center.y; }

bool Flame::is_gone() const { return gone; }

void Flame::update(float dt) {
    if (gone) {
        return;
    }
    float distance = speed * dt;
    center.x += direction.x * distance;
    center.y += direction.y * distance;
    pos = cell_of(center);
    range_left -= distance;
    if (Creature* target = touched()) {
        target->take_damage(damage);
        gone = true;
        return;
    }
    gone = range_left <= 0 || !location->is_walkable(pos);
}

Creature* Flame::touched() const {
    for (Creature* target: targets) {
        if (target->is_alive() && &target->get_location() == location &&
            std::hypot(target->get_x() - center.x, target->get_y() - center.y) < HIT_DISTANCE) {
            return target;
        }
    }
    return nullptr;
}
