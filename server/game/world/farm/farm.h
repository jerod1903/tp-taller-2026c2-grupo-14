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

// Una celda trabajada con la pala: Dirt si era pasto y se cavó una vez, Tilled si ya se puede plantar.
struct Plot {
    SoilType soil;
    Crop crop;
};

class Farm: public Location {
public:
    // La Farm es dueña de la Cabin. Va en unique_ptr porque una Location no se puede mover.
    // Enlaza las dos puertas: cabin_pos en la granja <-> door_pos en la cabaña.
    // tilled: los tiles tilled_soil del mapa, que arrancan cavados.
    Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin,
         const std::set<Position>& tilled = {});

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

    // Un golpe de pala: pasto -> dirt -> tilled. false si la celda no se puede cavar (fuera de la
    // grilla, tapada, otro terreno) o ya estaba tilled.
    bool dig(const Position& pos);
    // Planta en una celda tilled y vacía. false si no se puede.
    bool plant(const Position& pos, CropType type);
    // Cosecha un cultivo listo y deja la celda tilled y vacía. nullopt si no hay nada para cosechar.
    std::optional<CropHarvest> harvest(const Position& pos);
    // Las celdas trabajadas, por posición.
    const std::map<Position, Plot>& get_plots() const;
    // true si los plots cambiaron desde la última vez que se preguntó (para mandar el FarmState).
    bool take_changes();

    // Actualiza también la Cabin.
    void update(float dt) override;

private:
    Position cabin_pos;
    std::unique_ptr<Cabin> cabin;
    std::vector<std::unique_ptr<Animal>> animals;
    std::map<Position, Plot> plots;
    std::map<Position, ItemType> ground_items;
    uint32_t next_animal_id = 0;
    bool changed = false;

    int count_of(AnimalKind kind) const;
};

#endif  // FARM_H
