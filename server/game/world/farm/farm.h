#ifndef FARM_H
#define FARM_H

#include <cstdint>
#include <map>
#include <memory>
#include <optional>
#include <random>
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
#include "server/persistence/saved_game.h"

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
    int count_animals(AnimalKind kind) const;
    // Saca los primeros `quantity` de esa especie (los vendió). Lanza std::invalid_argument si no
    // hay tantos.
    void remove_animals(AnimalKind kind, int quantity);

    // Los animales y los cultivos cuentan un día más; lo que dejan los animales en el piso
    // queda en su celda.
    void start_new_day();

    // Un golpe de pala: pasto -> dirt -> tilled. false si la celda no se puede cavar (fuera de la
    // grilla, tapada, otro terreno) o ya estaba tilled.
    bool dig(const Position& pos);
    // Planta en una celda tilled y vacía. false si no se puede.
    bool plant(const Position& pos, CropType type);
    // Riega el cultivo de la celda. false si no hay o no lo necesitaba.
    bool water(const Position& pos);
    // La hora del día, en minutos de juego desde las 6:00: abre y cierra las ventanas de riego.
    void update_time(float minute_of_day);
    // Le da al jugador el cultivo listo de la celda, con su calidad: la celda queda vacía, o vuelve
    // a crecer si el cultivo vuelve a dar. Si no le entra en el inventario, no se cosecha.
    const Outcome& harvest(const Position& pos, Player& player);
    // Las celdas trabajadas, por posición.
    const std::map<Position, Plot>& get_plots() const;

    // Lo que se guarda: las celdas trabajadas, los objetos del piso y los animales.
    SavedFarm saved() const;
    // Reemplaza todo eso por lo guardado.
    void restore(const SavedFarm& saved);

    // Actualiza también la Cabin y levanta lo que dejaron los animales en el tick.
    void update(float dt) override;

private:
    Position cabin_pos;
    std::unique_ptr<Cabin> cabin;
    std::vector<std::unique_ptr<Animal>> animals;
    std::map<Position, Plot> plots;
    uint32_t next_animal_id = 0;
    float minute = 0.0f;  // la hora del día, para las ventanas de riego y la calidad
    std::mt19937 rng{std::random_device{}()};  // la suerte de cada cosecha

    // Pone en el piso, en su celda, lo que dejó cada animal.
    void collect_drops();
};

#endif  // FARM_H
