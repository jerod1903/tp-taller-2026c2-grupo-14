#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>
#include <optional>
#include <random>

#include "common/game_dto.h"
#include "server/game/characters/creature.h"

enum class AnimalKind { Cow, Chicken };

// Un animal de la granja: camina hacia un lado al azar (o se queda quieto) y cada tanto cambia.
// Cada especie es una subclase.
class Animal: public Creature {
public:
    Animal(uint32_t id, const Position& pos, Location& location, uint32_t seed);

    virtual AnimalKind get_kind() const = 0;
    // Si tiene algo para que el jugador le saque (la vaca, leche). Por defecto nunca.
    virtual bool is_ready() const;
    // Lo que deja en el piso de su celda al empezar el día (la gallina, un huevo), si deja algo.
    virtual std::optional<ItemType> start_new_day() = 0;

    void update(float dt) override;

private:
    float time_left = 0;  // segundos hasta elegir otra direccion
    std::mt19937 rng;

    void choose_direction();
};

#endif  // ANIMAL_H
