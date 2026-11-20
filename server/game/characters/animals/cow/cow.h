#ifndef COW_H
#define COW_H

#include <cstdint>
#include <random>

#include "server/game/characters/animals/animal.h"

// Se pone lista para ordeñar production_days días después de la última vez (o de llegar), y
// espera así hasta que la ordeñen.
class Cow: public Animal {
public:
    Cow(uint32_t id, const Position& pos, Location& location,
        uint32_t seed = std::random_device{}());

    AnimalKind get_kind() const override;
    bool is_ready() const override;
    void start_new_day() override;
    SavedAnimal saved() const override;
    void restore(const SavedAnimal& saved) override;
    // La deja sin leche y vuelve a contar los días.
    void milk();

private:
    int days_left;  // días hasta tener leche
    bool ready = false;
};

#endif  // COW_H
