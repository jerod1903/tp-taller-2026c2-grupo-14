#include "server/game/characters/enemies/skeleton.h"

#include <vector>

#include "server/config/server_config.h"

namespace {
// Lo que tarda en dar un espadazo y lo que queda dolorido por un golpe, en segundos
constexpr float ATTACK_TIME = 0.6f;
constexpr float HURT_TIME = 0.4f;
}  // namespace

Skeleton::Skeleton(uint32_t id, const Position& pos, Location& location,
                   const std::vector<Creature*>& visitors, uint32_t seed):
        Enemy(id, pos, location, server_config().enemies.skeleton.fighter, visitors, ATTACK_TIME,
              HURT_TIME, seed) {}

EntityType Skeleton::entity_type() const { return EntityType::Skeleton; }

void Skeleton::on_death() {
    set_direction(Direction::NONE);
    revive_left = server_config().enemies.skeleton.revive_seconds;
}

bool Skeleton::is_gone() const { return false; }

void Skeleton::while_dead(float dt) {
    revive_left -= dt;
    if (revive_left <= 0) {
        health = max_health;
    }
}
