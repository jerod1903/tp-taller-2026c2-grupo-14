#ifndef FARM_H
#define FARM_H

#include <memory>
#include <string>
#include <vector>

#include "server/game/characters/animals/animal.h"
#include "server/game/world/farm/cabin.h"
#include "server/game/world/location.h"
#include "server/game/world/position.h"

class Farm: public Location {
public:
    // La Farm es dueña de la Cabin. Va en unique_ptr porque una Location no se puede mover.
    // Enlaza las dos puertas: cabin_pos en la granja <-> door_pos en la cabaña.
    Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin);

    // Posición de la puerta de la cabaña en la grilla de la granja.
    const Position& get_cabin_pos() const;
    Cabin& get_cabin();

    // La granja es dueña de sus animales y los registra como entidades de la Location.
    void add_animal(AnimalKind kind, const Position& pos);
    const std::vector<std::unique_ptr<Animal>>& get_animals() const;

    // Actualiza también la Cabin.
    void update(float dt) override;

private:
    Position cabin_pos;
    std::unique_ptr<Cabin> cabin;
    std::vector<std::unique_ptr<Animal>> animals;
    uint32_t next_animal_id = 0;
};

#endif  // FARM_H
