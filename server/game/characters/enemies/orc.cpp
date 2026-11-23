#include "server/game/characters/enemies/orc.h"

#include <random>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/world/location.h"

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

const ServerConfig::Loot* Orc::loot_table() const { return &server_config().enemies.orc.loot; }

void Orc::on_death() {
    if (std::uniform_real_distribution<float>(0, 1)(rng) <
        server_config().enemies.orc.silver_chance) {
        get_location().drop(get_position(), ItemType::Silver);
    }
}
