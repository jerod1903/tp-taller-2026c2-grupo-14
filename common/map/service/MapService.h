#pragma once

#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"
#include "common/map/MapValidationIssue.h"
#include "common/map/persistence/MapRepository.h"

namespace stardew::maps {

struct MapLoadResult {
    MapDefinition map;
    std::vector<MapValidationIssue> issues;
};

// Coordina persistencia y validacion sin depender de Qt ni del formato TOML.
class MapService {
public:
    MapService(const MapRepository& repository, MapRequirements requirements);

    MapLoadResult loadMap(const std::string& path) const;
    std::vector<MapValidationIssue> validate(const MapDefinition& map) const;
    // Guarda siempre y devuelve los problemas que encontro el validador (vacio si es valido)
    std::vector<MapValidationIssue> saveMap(const std::string& path,
                                            const MapDefinition& map) const;
    const MapRequirements& requirements() const;

private:
    const MapRepository& repository_;
    MapRequirements requirements_;
};

}  // namespace stardew::maps
