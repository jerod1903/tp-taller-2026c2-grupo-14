#pragma once

#include <optional>
#include <vector>

#include "common/map/MapDefinition.h"

namespace stardew::maps {

struct AreaRequirement {
    AreaType type{};
    int count = 0;
    std::vector<int> floorNumbers;
};

struct ElementRequirement {
    AreaType areaType{};
    ElementType type{};
    ElementLayer layer{};
};

struct TransitionRequirement {
    // Un piso ausente aplica la regla a todos los pisos del tipo indicado.
    AreaType fromAreaType{};
    std::optional<int> fromFloor;
    AreaType toAreaType{};
    std::optional<int> toFloor;
};

struct MapRequirements {
    int formatVersion = 0;
    bool allowStructureOverlap = false;
    // Exige al menos una ruta valida de salida en cada area.
    bool requireOutgoingTransitionForEachArea = false;
    std::vector<AreaRequirement> areas;
    std::vector<ElementRequirement> elements;
    std::vector<TransitionRequirement> transitions;
};

}  // namespace stardew::maps
