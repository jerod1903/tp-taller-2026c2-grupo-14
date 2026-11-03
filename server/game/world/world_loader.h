#ifndef WORLD_LOADER_H
#define WORLD_LOADER_H

#include <string>
#include <vector>

#include "common/lobby_dto.h"
#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

// Un mundo que se puede elegir al crear una partida. El worldId es el nombre del archivo.
struct GameWorld {
    WorldInfo info;
    stardew::maps::MapDefinition map;
};

// Carga los *.toml de maps_dir (sin subcarpetas), ordenados por worldId. Un mapa que no se puede
// leer, no pasa la validación o no arma un World se saltea con un warning.
std::vector<GameWorld> load_worlds(const std::string& maps_dir,
                                   const stardew::maps::MapRequirements& requirements);

#endif  // WORLD_LOADER_H
