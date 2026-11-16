#ifndef WORLD_BUILDER_H
#define WORLD_BUILDER_H

#include <memory>

#include "common/map/MapDefinition.h"
#include "server/game/world/world.h"

// Arma el World de una partida a partir de un mapa ya validado con MapService. Lanza
// std::invalid_argument si al mapa le falta algo que el World necesita.
std::unique_ptr<World> build_world(const stardew::maps::MapDefinition& map);

#endif  // WORLD_BUILDER_H
