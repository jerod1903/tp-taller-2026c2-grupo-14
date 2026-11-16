#include "server/game/characters/creature.h"

#include <algorithm>
#include <cmath>

#include "common/game_dto.h"
#include "server/game/world/location.h"

namespace {
constexpr float CELL_CENTER = 0.5f;
// Tramo máximo por paso de la simulación: menos de una celda, así nunca atraviesa una pared
// aunque dt sea grande.
constexpr float MAX_SUBSTEP = 0.25f;
}  // namespace

Creature::Creature(uint32_t id, const Position& pos, Location& location, int health,
                   float base_speed):
        Entity(id, pos, location),
        health(health),
        base_speed(base_speed),
        x(pos.x + CELL_CENTER),
        y(pos.y + CELL_CENTER) {}

int Creature::get_health() const { return health; }

void Creature::take_damage(int amount) { health = std::max(0, health - amount); }

bool Creature::is_alive() const { return health > 0; }

float Creature::speed() const { return base_speed; }

void Creature::set_direction(Direction dir) { direction = dir; }

Direction Creature::get_direction() const { return direction; }

float Creature::get_x() const { return x; }

float Creature::get_y() const { return y; }

bool Creature::has_stepped() const { return stepped; }

bool Creature::is_trapped() const { return !fits(x, y); }

void Creature::relocate(Location& target, const Position& arrival) {
    Entity::relocate(target, arrival);
    x = arrival.x + CELL_CENTER;
    y = arrival.y + CELL_CENTER;
}

void Creature::place_at(const Point& point) {
    x = point.x;
    y = point.y;
    pos = cell_of(point);
}

bool Creature::advance(float dt) {
    stepped = false;
    Position delta = step(Position{0, 0}, direction);
    if (delta.x == 0 && delta.y == 0) {
        return false;
    }
    float distance = speed() * dt;
    if (delta.x != 0 && delta.y != 0) {
        distance /= std::sqrt(2.0f);  // lo que avanza en cada eje, para ir a la misma velocidad
    }
    int substeps = std::max(1, static_cast<int>(std::ceil(distance / MAX_SUBSTEP)));
    float substep = distance / substeps;
    for (int i = 0; i < substeps; ++i) {
        move_x(delta.x * substep);
        move_y(delta.y * substep);
    }
    Position cell{static_cast<int>(std::floor(x)), static_cast<int>(std::floor(y))};
    if (cell != pos) {
        pos = cell;
        stepped = true;
    }
    return stepped;
}

void Creature::move_x(float dx) {
    float target = x + dx;
    if (dx == 0.0f || fits(target, y)) {
        x = target;
        return;
    }
    // Pegado a la pared: el borde de la caja queda justo en el borde de la celda bloqueada
    if (dx > 0) {
        x = std::ceil(target + CHARACTER_HALF_WIDTH) - 1 - CHARACTER_HALF_WIDTH;
    } else {
        x = std::floor(target - CHARACTER_HALF_WIDTH) + 1 + CHARACTER_HALF_WIDTH;
    }
}

void Creature::move_y(float dy) {
    float target = y + dy;
    if (dy == 0.0f || fits(x, target)) {
        y = target;
        return;
    }
    if (dy > 0) {
        y = std::ceil(target + CHARACTER_HALF_HEIGHT) - 1 - CHARACTER_HALF_HEIGHT;
    } else {
        y = std::floor(target - CHARACTER_HALF_HEIGHT) + 1 + CHARACTER_HALF_HEIGHT;
    }
}

bool Creature::fits(float cx, float cy) const {
    // El borde derecho y el de abajo no cuentan: una caja que termina justo en una celda no la pisa
    int left = static_cast<int>(std::floor(cx - CHARACTER_HALF_WIDTH));
    int right = static_cast<int>(std::ceil(cx + CHARACTER_HALF_WIDTH)) - 1;
    int top = static_cast<int>(std::floor(cy - CHARACTER_HALF_HEIGHT));
    int bottom = static_cast<int>(std::ceil(cy + CHARACTER_HALF_HEIGHT)) - 1;
    for (int row = top; row <= bottom; ++row) {
        for (int column = left; column <= right; ++column) {
            if (!location->is_walkable(Position{column, row})) {
                return false;
            }
        }
    }
    return true;
}
