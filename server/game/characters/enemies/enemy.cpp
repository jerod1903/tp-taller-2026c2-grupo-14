#include "server/game/characters/enemies/enemy.h"

#include <cmath>
#include <cstdlib>
#include <vector>

#include "server/game/world/direction.h"
#include "server/game/world/location.h"

Enemy::Enemy(uint32_t id, const Position& pos, Location& location,
             const ServerConfig::Fighter& stats, const std::vector<Creature*>& visitors,
             float attack_time, float hurt_time):
        Creature(id, pos, location, stats.health, stats.speed, stats.damage),
        max_health(stats.health),
        sight(stats.sight),
        reach(stats.reach),
        cooldown(stats.cooldown),
        attack_time(attack_time),
        hurt_time(hurt_time),
        visitors(visitors) {}

void Enemy::take_damage(int amount) {
    bool was_alive = is_alive();
    Creature::take_damage(amount);
    if (is_alive()) {
        hurt_left = hurt_time;
        return;
    }
    // Muerto no sigue pegando ni dolorido: al revivir arranca sin nada
    attacking_left = 0;
    hurt_left = 0;
    if (was_alive) {
        on_death();
    }
}

EntityAction Enemy::action() const {
    if (!is_alive()) {
        return EntityAction::Dead;
    }
    if (hurt_left > 0) {
        return EntityAction::Hurt;
    }
    if (attacking_left > 0) {
        return EntityAction::Attacking;
    }
    return EntityAction::None;
}

void Enemy::on_death() {}

bool Enemy::is_gone() const { return !is_alive(); }

void Enemy::update(float dt) {
    if (!is_alive()) {
        return;
    }
    cooldown_left -= dt;
    attacking_left -= dt;
    hurt_left -= dt;
    react();
    advance(dt);
}

void Enemy::react() {
    Creature* target = nearest_visible();
    if (target == nullptr) {
        set_direction(Direction::NONE);
        return;
    }
    if (distance_to(*target) > reach) {
        set_direction(direction_towards(target->get_x() - x, target->get_y() - y));
        return;
    }
    set_direction(Direction::NONE);
    if (cooldown_left <= 0) {
        attack(*target);
        cooldown_left = cooldown;
        attacking_left = attack_time;
    }
}

Creature* Enemy::nearest_visible() const {
    Creature* nearest = nullptr;
    for (Creature* visitor: visitors) {
        if (visitor->is_alive() && sees(*visitor) &&
            (nearest == nullptr || distance_to(*visitor) < distance_to(*nearest))) {
            nearest = visitor;
        }
    }
    return nearest;
}

bool Enemy::sees(const Creature& target) const {
    return &target.get_location() == location && distance_to(target) <= sight &&
           clear_line_to(target);
}

bool Enemy::clear_line_to(const Creature& target) const {
    // Avanza de a una celda desde la del enemigo hasta la del jugador, por las que cruza la recta
    // entre las dos, y corta si alguna es pared. `error` acumula cuánto se aleja el recorrido de
    // la recta: según su valor, el próximo paso va en x, en y o en los dos (diagonal).
    Position from = get_position();
    const Position& to = target.get_position();
    int dx = std::abs(to.x - from.x);
    int dy = -std::abs(to.y - from.y);
    int step_x = from.x < to.x ? 1 : -1;
    int step_y = from.y < to.y ? 1 : -1;
    int error = dx + dy;
    while (from != to) {
        if (!location->is_walkable(from)) {
            return false;
        }
        int doubled = 2 * error;
        if (doubled >= dy) {
            error += dy;
            from.x += step_x;
        }
        if (doubled <= dx) {
            error += dx;
            from.y += step_y;
        }
    }
    return true;
}

float Enemy::distance_to(const Creature& target) const {
    return std::hypot(target.get_x() - x, target.get_y() - y);
}
