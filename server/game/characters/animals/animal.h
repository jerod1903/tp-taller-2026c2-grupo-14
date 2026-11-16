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

    // Solo la vaca se pone lista para ordeñar, COW_READY_SECONDS despues de la ultima vez (o de
    // aparecer). La gallina nunca.
    bool is_ready() const;
    // Deja a la vaca sin leche y vuelve a contar desde COW_READY_SECONDS.
    void milk();

    void update(float dt) override;

private:
    AnimalKind kind;
    float time_left = 0;  // segundos hasta elegir otra direccion
    float time_to_ready;  // segundos hasta tener leche (solo cuenta en la vaca)
    bool ready = false;
    std::mt19937 rng;

    void choose_direction();
};

#endif  // ANIMAL_H
