#ifndef ANIMAL_H
#define ANIMAL_H

#include <cstdint>
#include <optional>
#include <random>

#include "common/game_dto.h"
#include "server/game/characters/animals/animal_kind.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/wanderer.h"
#include "server/persistence/saved_game.h"

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

    // Lo que se guarda de él: su especie, su celda y cómo va su producción.
    virtual SavedAnimal saved() const = 0;
    // Vuelve a producir como en `saved` (la especie y la celda las pone quien lo crea).
    virtual void restore(const SavedAnimal& saved) = 0;

    void update(float dt) override;

protected:
    std::mt19937 rng;

private:
    Wanderer wanderer;
};

#endif  // ANIMAL_H
