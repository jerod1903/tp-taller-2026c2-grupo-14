#include "server/game/characters/enemies/vampire.h"

#include <algorithm>
#include <vector>

#include "server/config/server_config.h"

namespace {
// Cuánto tardan en dar un golpe y cuánto quedan doloridos por uno, en segundos
constexpr float ATTACK_TIME = 0.8f;
constexpr float HURT_TIME = 0.4f;
// En frenesí duplica su habilidad
constexpr int FRENZY = 2;

const ServerConfig::Vampire& config() { return server_config().enemies.vampire; }

// Los valores de la pareja, con lo propio de cada uno
ServerConfig::Fighter stats_of(VampireKind kind) {
    ServerConfig::Fighter stats = config().fighter;
    if (kind == VampireKind::Vampiress) {
        stats.speed = config().fast_speed;
    } else {
        stats.damage = config().strong_damage;
    }
    return stats;
}
}  // namespace

Vampire::Vampire(uint32_t id, const Position& pos, Location& location,
                 const std::vector<Creature*>& visitors, VampireKind kind, uint32_t seed):
        Enemy(id, pos, location, stats_of(kind), visitors, ATTACK_TIME, HURT_TIME, seed),
        kind(kind) {}

void Vampire::pair_with(Vampire& other) {
    partner = &other;
    other.partner = this;
}

bool Vampire::is_in_frenzy() const { return frenzy; }

void Vampire::partner_died() { frenzy = is_alive(); }

void Vampire::partner_revived() { frenzy = false; }

EntityType Vampire::entity_type() const {
    return kind == VampireKind::Vampiress ? EntityType::Vampiress : EntityType::Vampire;
}

void Vampire::on_death() {
    set_direction(Direction::NONE);
    frenzy = false;
    partner->partner_died();
}

bool Vampire::is_gone() const { return false; }

void Vampire::while_dead(float dt) {
    if (!revive_left && badly_hurt_player_nearby()) {
        revive_left = config().revive_seconds;
    }
    if (revive_left && (*revive_left -= dt) <= 0) {
        revive();
    }
}

float Vampire::speed() const {
    bool faster = frenzy && kind == VampireKind::Vampiress;
    return faster ? base_speed * FRENZY : base_speed;
}

int Vampire::attack_damage() const {
    bool stronger = frenzy && kind == VampireKind::Vampire;
    return stronger ? damage * FRENZY : damage;
}

bool Vampire::badly_hurt_player_nearby() const {
    const std::vector<Creature*>& visitors = get_visitors();
    return std::any_of(visitors.begin(), visitors.end(), [this](const Creature* visitor) {
        return &visitor->get_location() == location && visitor->is_badly_hurt() &&
               distance_to(*visitor) <= config().blood_reach;
    });
}

void Vampire::revive() {
    health = max_health;
    revive_left.reset();
    frenzy = !partner->is_alive();
    partner->partner_revived();
}

const ServerConfig::Loot* Vampire::loot_table() const {
    return &server_config().enemies.vampire.loot;
}
