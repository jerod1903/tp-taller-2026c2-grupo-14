#ifndef TEST_WORLDS_H
#define TEST_WORLDS_H

#include <vector>

#include "common/lobby_dto.h"
#include "common/map/MapDefinition.h"
#include "common/map/persistence/MapRepository.h"
#include "common/map/persistence/MapRequirementsLoader.h"
#include "server/game/world/world_loader.h"

// El mapa de ejemplo del repo, que es válido: se lee una sola vez.
inline const stardew::maps::MapDefinition& example_map() {
    static const stardew::maps::MapDefinition map =
            stardew::maps::TomlMapRepository().loadMap(TALLER_MAPS_DIR "/valle.toml");
    return map;
}

// Las reglas de los mapas del repo: se leen una sola vez.
inline const stardew::maps::MapRequirements& example_requirements() {
    static const stardew::maps::MapRequirements requirements =
            stardew::maps::loadMapRequirements(TALLER_MAPS_DIR "/config/map_requirements.toml");
    return requirements;
}

// Mundos con esos ids y nombres, todos con el mapa de ejemplo.
inline std::vector<GameWorld> test_worlds(const std::vector<WorldInfo>& infos) {
    std::vector<GameWorld> worlds;
    for (const WorldInfo& info: infos) {
        worlds.push_back(GameWorld{info, example_map()});
    }
    return worlds;
}

#endif  // TEST_WORLDS_H
