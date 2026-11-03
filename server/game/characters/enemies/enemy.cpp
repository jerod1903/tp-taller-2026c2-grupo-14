#include "server/game/characters/enemies/enemy.h"

Enemy::Enemy(uint32_t id, const Position& pos, Location& location, int health, float base_speed):
        Creature(id, pos, location, health, base_speed) {}

void Enemy::take_damage(int amount) {
    bool was_alive = is_alive();
    Creature::take_damage(amount);
    if (was_alive && !is_alive()) {
        on_death();
    }
}

void Enemy::on_death() {}
