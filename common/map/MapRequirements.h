#pragma once

#include <optional>
#include <vector>

#include "common/map/MapDefinition.h"

namespace stardew::maps {

struct AreaRequirement {
    AreaType type{};
    int count = 0;
    std::vector<int> levelNumbers;
};

// Donde puede ir un tipo de elemento, cuanto mide y cuantos puede haber. Un tipo que puede ir en
// varios tipos de area tiene una regla por cada uno.
struct ElementRule {
    AreaType areaType{};
    ElementType type{};
    ElementLayer layer{};
    int width = 1;
    int height = 1;
    // Sin valor: no hay limite
    std::optional<int> maxPerArea;
    // Tiene que haber al menos uno en alguna area de ese tipo
    bool required = false;
};

// Los tiles que se pueden pintar en un tipo de area. Un tipo de area sin regla acepta cualquiera.
struct TileRule {
    AreaType areaType{};
    std::vector<TileType> types;
};

struct TransitionRequirement {
    // Un piso ausente aplica la regla a todos los pisos del tipo indicado.
    AreaType fromAreaType{};
    std::optional<int> fromLevel;
    AreaType toAreaType{};
    std::optional<int> toLevel;
};

struct MapRequirements {
    int formatVersion = 0;
    bool allowStructureOverlap = false;
    // Exige al menos una ruta valida de salida en cada area.
    bool requireOutgoingTransitionForEachArea = false;
    std::vector<AreaRequirement> areas;
    std::vector<ElementRule> elements;
    std::vector<TransitionRequirement> transitions;
    std::vector<TileRule> tiles;
};

}  // namespace stardew::maps
