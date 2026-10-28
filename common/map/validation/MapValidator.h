#pragma once

#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"
#include "common/map/MapValidationIssue.h"

namespace stardew::maps {

std::vector<MapValidationIssue> validateMap(const MapDefinition& map,
                                            const MapRequirements& requirements);

}  // namespace stardew::maps
