#include "server/game/characters/enemies/skull.h"

#include <algorithm>
#include <memory>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/dungeon_level.h"

namespace {
// Lo que dura el gesto de tirar una llama. No tiene dolor: no le pasa nada.
constexpr float ATTACK_TIME = 0.4f;
constexpr float HURT_TIME = 0.0f;

// La vida y el daño no se usan: no se la puede matar y quema con sus llamas. Alcanza hasta donde
// ve.
ServerConfig::Fighter stats() {
    const ServerConfig::Skull& config = server_config().enemies.skull;
    return ServerConfig::Fighter{1, 0, config.speed, config.sight, config.sight, config.cooldown};
}
}  // namespace

Skull::Skull(uint32_t id, const Position& pos, DungeonLevel& level,
             const std::vector<Creature*>& visitors, FlameKind kind, uint32_t seed):
        Enemy(id, pos, level, stats(), visitors, ATTACK_TIME, HURT_TIME, seed),
        level(level),
        kind(kind) {}

EntityType Skull::entity_type() const {
    return kind == FlameKind::Green ? EntityType::GreenSkull : EntityType::Skull;
}

void Skull::take_damage(int) {}

void Skull::update(float dt) {
    Enemy::update(dt);
    for (const std::unique_ptr<Flame>& flame: flames) {
        flame->update(dt);
    }
    flames.erase(
            std::remove_if(flames.begin(), flames.end(),
                           [](const std::unique_ptr<Flame>& flame) { return flame->is_gone(); }),
            flames.end());
}

const std::vector<std::unique_ptr<Flame>>& Skull::get_flames() const { return flames; }

void Skull::strike(Creature& target) {
    flames.push_back(std::make_unique<Flame>(level.new_flame_id(), Point{x, y},
                                             Point{target.get_x(), target.get_y()}, level,
                                             get_visitors(), kind));
}
