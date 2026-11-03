#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>

#include "server/game/characters/creature.h"

class Enemy: public Creature {
public:
    Enemy(uint32_t id, const Position& pos, Location& location, int health, float base_speed);

    // Llama a on_death() la primera vez que la vida llega a 0.
    void take_damage(int amount) override;

    // Drops, experiencia, etc. Por defecto no hace nada.
    virtual void on_death();
};

#endif  // ENEMY_H
