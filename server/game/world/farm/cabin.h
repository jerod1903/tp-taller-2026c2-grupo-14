#ifndef CABIN_H
#define CABIN_H

#include <cstdint>
#include <map>
#include <string>
#include <vector>

#include "server/game/world/location.h"
#include "server/game/world/position.h"
#include "server/game/world/spawn_point.h"

// El interior de la cabaña. Es una Location propia, pero vive dentro de la Farm.
class Cabin: public Location {
public:
    // bed_slots: los tiles sleepable, en el orden en que se ocupan.
    Cabin(std::string name, Grid grid, const Position& door_pos,
          std::vector<Position> bed_slots = {});

    // Tile interior que lleva de vuelta a la granja.
    const Position& get_door_pos() const;

    // La cama del jugador: la primera libre la primera vez, después siempre la misma (aunque se
    // vaya y vuelva). Lanza std::logic_error si no queda ningún lugar.
    const Position& assign_bed(uint32_t player_id);
    // Donde aparece el jugador: acostado en el medio de su cama, con la frazada encima. Le asigna
    // una si no tenia.
    SpawnPoint bed_spawn(uint32_t player_id);
    // Las camas que ya tienen dueño, por player_id.
    const std::map<uint32_t, Position>& get_beds() const;
    std::size_t bed_slot_count() const;

    // Como cualquier Location, pero la cabecera de una cama con dueño no se pisa.
    bool is_walkable(const Position& pos) const override;

    void update(float dt) override;

private:
    Position door_pos;
    std::vector<Position> bed_slots;
    std::map<uint32_t, Position> beds;

    bool is_bed_head(const Position& pos) const;
};

#endif  // CABIN_H
