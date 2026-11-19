#ifndef ORC_H
#define ORC_H

#include <cstdint>
#include <vector>

#include "server/game/characters/enemies/enemy.h"

// Pelea cuerpo a cuerpo y aparece en grupo (OrcSpawner). Al morir no revive. Sus valores salen de
// [enemies.orc] en server_config.toml.
class Orc: public Enemy {
public:
    Orc(uint32_t id, const Position& pos, Location& location,
        const std::vector<Creature*>& visitors, uint32_t seed);

    EntityType entity_type() const override;
};

#endif  // ORC_H
