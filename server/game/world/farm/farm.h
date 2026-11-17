#ifndef FARM_H
#define FARM_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "common/farm_dto.h"
#include "server/game/characters/animals/animal.h"
#include "server/game/world/farm/cabin.h"
#include "server/game/world/farm/crop.h"
#include "server/game/world/location.h"
#include "server/game/world/position.h"
#include "server/game/world/spawn_point.h"

class Farm: public Location {
public:
    // La Farm es dueña de la Cabin. Va en unique_ptr porque una Location no se puede mover.
    // Enlaza las dos puertas: cabin_pos en la granja <-> door_pos en la cabaña.
    // plots: los tiles tilled_soil, donde se puede cavar.
    Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin,
         std::set<Position> plots = {});

    // Posición de la puerta de la cabaña en la grilla de la granja.
    const Position& get_cabin_pos() const;
    Cabin& get_cabin();
    const Cabin& get_cabin() const;

    // La cama del jugador en la cabaña.
    SpawnPoint bed_spawn(uint32_t player_id);

    // La granja es dueña de sus animales y los registra como entidades de la Location.
    // false si ya tiene el máximo de esa especie (max_per_farm).
    bool add_animal(AnimalKind kind, const Position& pos);
    const std::vector<std::unique_ptr<Animal>>& get_animals() const;

    // Los objetos tirados en el piso (los huevos), uno por celda.
    const std::map<Position, ItemType>& get_ground_items() const;
    // Levanta el objeto de la celda, si hay uno.
    std::optional<ItemType> pick_up(const Position& pos);

    // Los animales producen; lo que dejan en el piso queda en su celda.
    void start_new_day();

    // Cava la celda. false si no es un plot o ya estaba cavada.
    bool till(const Position& pos);
    // Planta en una celda cavada y vacía. false si no se puede.
    bool plant(const Position& pos, CropType type);
    // Cosecha un cultivo listo y deja la celda cavada y vacía. nullopt si no hay nada para cosechar.
    std::optional<CropHarvest> harvest(const Position& pos);
    // Las celdas cavadas, por posición.
    const std::map<Position, Crop>& get_crops() const;

    // Actualiza también la Cabin.
    void update(float dt) override;

private:
    Position cabin_pos;
    std::unique_ptr<Cabin> cabin;
    std::vector<std::unique_ptr<Animal>> animals;
    std::set<Position> plots;
    std::map<Position, Crop> crops;
    std::map<Position, ItemType> ground_items;
    uint32_t next_animal_id = 0;

    int count_of(AnimalKind kind) const;
};

#endif  // FARM_H
