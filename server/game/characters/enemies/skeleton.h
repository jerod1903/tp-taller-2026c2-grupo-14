#ifndef SKELETON_H
#define SKELETON_H

#include <cstdint>
#include <vector>

#include "server/game/characters/enemies/enemy.h"

// Pelea cuerpo a cuerpo y aparece solo. Al morir queda tirado y revive pasado un tiempo, con la
// vida llena, en el mismo lugar. Sus valores salen de [enemies.skeleton] en server_config.toml.
class Skeleton: public Enemy {
public:
    Skeleton(uint32_t id, const Position& pos, Location& location,
             const std::vector<Creature*>& visitors, uint32_t seed);

    EntityType entity_type() const override;
    void on_death() override;
    // Nunca se va: siempre revive.
    bool is_gone() const override;

protected:
    // Cuenta el tiempo para revivir.
    void while_dead(float dt) override;

private:
    float revive_left = 0;

    const ServerConfig::Loot* loot_table() const override;
};

#endif  // SKELETON_H
