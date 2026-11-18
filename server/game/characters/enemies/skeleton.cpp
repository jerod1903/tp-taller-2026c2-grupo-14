#include "server/game/characters/enemies/skeleton.h"

#include <vector>

#include "server/config/server_config.h"

Skeleton::Skeleton(uint32_t id, const Position& pos, Location& location,
                   const std::vector<Creature*>& visitors):
        Enemy(id, pos, location, server_config().enemies.skeleton.fighter, visitors) {}

void Skeleton::on_death() {
    set_direction(Direction::NONE);
    revive_left = server_config().enemies.skeleton.revive_seconds;
}

bool Skeleton::is_gone() const { return false; }

void Skeleton::update(float dt) {
    if (is_alive()) {
        Enemy::update(dt);
        return;
    }
    revive_left -= dt;
    if (revive_left <= 0) {
        health = max_health;
    }
}
