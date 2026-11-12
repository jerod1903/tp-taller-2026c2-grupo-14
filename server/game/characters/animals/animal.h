#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>
#include <random>

#include "server/game/characters/creature.h"

enum class AnimalKind { Cow, Chicken };

// Un animal de la granja: camina hacia un lado al azar (o se queda quieto) y cada tanto cambia.
class Animal: public Creature {
public:
    Animal(uint32_t id, AnimalKind kind, const Position& pos, Location& location,
           uint32_t seed = std::random_device{}());

    AnimalKind get_kind() const;

    void update(float dt) override;

private:
    AnimalKind kind;
    float time_left = 0;  // segundos hasta elegir otra direccion
    std::mt19937 rng;

    void choose_direction();
};

#endif  // ANIMAL_H
