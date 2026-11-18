#ifndef CHICKEN_H
#define CHICKEN_H

#include <cstdint>
#include <optional>
#include <random>

#include "server/game/characters/animals/animal.h"

// Pone un huevo en el piso cada production_days días, a una hora al azar del día, sin que nadie
// haga nada.
class Chicken: public Animal {
public:
    Chicken(uint32_t id, const Position& pos, Location& location,
            uint32_t seed = std::random_device{}());

    AnimalKind get_kind() const override;
    // Si el día terminó antes de que pusiera el que le tocaba, lo pone ahora.
    void start_new_day() override;
    std::optional<ItemType> take_drop() override;
    void update(float dt) override;

private:
    int days_left;                // días hasta el próximo huevo
    std::optional<float> lay_in;  // segundos hasta poner el de hoy, si hoy pone
    bool laid = false;            // puso y la granja todavía no lo levantó

    void schedule_egg();
};

#endif  // CHICKEN_H
