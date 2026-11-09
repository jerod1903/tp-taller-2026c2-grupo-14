#include "common/map/service/MapService.h"

#include <utility>

#include "common/map/validation/MapValidator.h"

namespace stardew::maps {

MapService::MapService(const MapRepository& repository, MapRequirements requirements):
        repository_(repository), requirements_(std::move(requirements)) {}

MapLoadResult MapService::loadMap(const std::string& path) const {
    // Se conserva el mapa aunque tenga errores semanticos para permitir corregirlo en el editor.
    auto map = repository_.loadMap(path);
    auto issues = validate(map);
    return {std::move(map), std::move(issues)};
}

std::vector<MapValidationIssue> MapService::validate(const MapDefinition& map) const {
    return validateMap(map, requirements_);
}

std::vector<MapValidationIssue> MapService::saveMap(const std::string& path,
                                                    const MapDefinition& map) const {
    auto issues = validate(map);
    if (!issues.empty()) {
        return issues;
    }

    repository_.saveMap(path, map);
    return {};
}

const MapRequirements& MapService::requirements() const { return requirements_; }

}  // namespace stardew::maps
