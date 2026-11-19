#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>
#include <optional>
#include <random>

#include "common/game_dto.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/wanderer.h"

enum class AnimalKind { Cow, Chicken };

// Un animal de la granja: deambula (camina hacia un lado al azar o se queda quieto, y cada tanto
// cambia). Cada especie es una subclase.
class Animal: public Creature {
public:
    Animal(uint32_t id, const Position& pos, Location& location, uint32_t seed);

    virtual AnimalKind get_kind() const = 0;
    // Si tiene algo para que el jugador le saque (la vaca, leche). Por defecto nunca.
    virtual bool is_ready() const;
    virtual void start_new_day() = 0;
    // Lo que dejó en el piso desde la última vez que se preguntó (la gallina, un huevo).
    virtual std::optional<ItemType> take_drop();

    void update(float dt) override;

protected:
    std::mt19937 rng;

private:
    Wanderer wanderer;
};

#endif  // ANIMAL_H
