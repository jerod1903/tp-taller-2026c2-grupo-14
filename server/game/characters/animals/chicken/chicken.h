#ifndef CHICKEN_H
#define CHICKEN_H

#include <cstdint>
#include <optional>
#include <random>

#include "server/game/characters/animals/animal.h"

// Pone un huevo en el piso cada production_days días, sin que nadie haga nada.
class Chicken: public Animal {
public:
    Chicken(uint32_t id, const Position& pos, Location& location,
            uint32_t seed = std::random_device{}());

    AnimalKind get_kind() const override;
    std::optional<ItemType> start_new_day() override;

private:
    int days_left;  // días hasta el próximo huevo
};

#endif  // CHICKEN_H
