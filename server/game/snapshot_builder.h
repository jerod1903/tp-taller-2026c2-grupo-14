#ifndef SNAPSHOT_BUILDER_H
#define SNAPSHOT_BUILDER_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/world/world.h"

// Pasa el estado del World a lo que viaja al cliente: posiciones en milésimas de celda, una
// entidad por jugador, animal, enemigo y cama con dueño.
class SnapshotBuilder {
public:
    static Snapshot build(uint32_t tick, const World& world);
    // Una celda por cada plot y un objeto por cada cosa tirada en el piso de la granja. Se manda
    // solo cuando cambia, no en cada tick.
    static FarmState farm_state(const Farm& farm);
};

#endif  // SNAPSHOT_BUILDER_H
