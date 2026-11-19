#include "server/game/characters/enemies/orc.h"

#include <vector>

#include "server/config/server_config.h"

namespace {
// Lo que tarda en dar un golpe y lo que queda dolorido, en segundos
constexpr float ATTACK_TIME = 0.6f;
constexpr float HURT_TIME = 0.4f;
}  // namespace

Orc::Orc(uint32_t id, const Position& pos, Location& location,
         const std::vector<Creature*>& visitors, uint32_t seed):
        Enemy(id, pos, location, server_config().enemies.orc.fighter, visitors, ATTACK_TIME,
              HURT_TIME, seed) {}

EntityType Orc::entity_type() const { return EntityType::Orc; }
