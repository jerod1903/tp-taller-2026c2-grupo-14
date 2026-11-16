#ifndef SNAPSHOT_BUILDER_H
#define SNAPSHOT_BUILDER_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/world/world.h"

// Pasa el estado del World a lo que viaja al cliente: posiciones en milésimas de celda, una
// entidad por jugador, animal y cama con dueño.
class SnapshotBuilder {
public:
    static Snapshot build(uint32_t tick, const World& world);
};

#endif  // SNAPSHOT_BUILDER_H
