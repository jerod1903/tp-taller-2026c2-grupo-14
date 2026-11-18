#ifndef WANDERER_H
#define WANDERER_H

#include <random>

#include "server/game/characters/creature.h"

// Hace deambular a una criatura: la pone a caminar hacia un lado al azar (o la deja quieta) y
// cada tanto, entre `min_walk` y `max_walk` segundos, cambia.
class Wanderer {
public:
    // `rng` es el de la criatura: tiene que vivir más que el Wanderer.
    Wanderer(std::mt19937& rng, float min_walk, float max_walk);

    // Si ya pasó su tiempo, le elige otra dirección. No la mueve: eso lo hace la criatura.
    void wander(Creature& creature, float dt);

private:
    std::mt19937& rng;
    float min_walk;
    float max_walk;
    float time_left = 0;  // segundos hasta elegir otra direccion
};

#endif  // WANDERER_H
