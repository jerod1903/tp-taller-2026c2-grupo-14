#include "common/map/validation/MapValidator.h"

#include <algorithm>
#include <map>
#include <optional>
#include <set>
#include <string>
#include <vector>

#include "common/map/MapSprites.h"

namespace stardew::maps {
namespace {

using Issues = std::vector<MapValidationIssue>;

// Las areas bien declaradas, para que las validaciones siguientes resuelvan referencias.
struct AreaIndex {
    std::map<std::string, const MapArea*> byId;
    std::map<std::string, AreaType> typeById;
    std::map<std::string, std::optional<int>> levelById;
};

// Los elementos con id unico y los que ademas caen dentro de su area.
struct ElementIndex {
    std::map<std::string, const MapElement*> byId;
    std::map<std::string, std::vector<const MapElement*>> byArea;
    std::set<const MapElement*> valid;
};

using TransitionsByArea = std::map<std::string, std::vector<const MapTransition*>>;

const AreaRequirement* findAreaRequirement(const MapRequirements& requirements, AreaType type) {
    const auto it = std::find_if(
            requirements.areas.begin(), requirements.areas.end(),
            [type](const AreaRequirement& requirement) { return requirement.type == type; });
    return it == requirements.areas.end() ? nullptr : &*it;
}

// Comprueba si dos estructuras ocupan alguna de las mismas celdas.
bool rectanglesOverlap(const MapElement& first, const MapElement& second) {
    return first.x < second.x + second.width && second.x < first.x + first.width &&
           first.y < second.y + second.height && second.y < first.y + first.height;
}

// Las estructuras y los NPCs tapan sus celdas, salvo los que llevan a otra area.
bool blocksCell(const MapElement& element, const std::set<std::string>& triggerIds, int x, int y) {
    return isSolid(element) && triggerIds.find(element.id) == triggerIds.end() && x >= element.x &&
           x < element.x + element.width && y >= element.y && y < element.y + element.height;
}

void validateHeader(const MapDefinition& map, const MapRequirements& requirements, Issues& errors) {
    if (!map.formatVersion.has_value()) {
        errors.push_back({.code = MapIssueCode::MissingFormatVersion});
    } else if (*map.formatVersion != requirements.formatVersion) {
        errors.push_back({
                .code = MapIssueCode::UnsupportedFormatVersion,
                .expected = requirements.formatVersion,
                .actual = *map.formatVersion,
        });
    }
    if (map.name.empty()) {
        errors.push_back({.code = MapIssueCode::MissingMapName});
    }
}

// Comprueba los numeros de piso usando las reglas configuradas para cada tipo de area.
void validateLevelNumber(const MapArea& area, const MapRequirements& requirements,
                         std::map<AreaType, std::set<int>>& levelNumbersByType, Issues& errors) {
    const auto* requirement = findAreaRequirement(requirements, *area.type);
    if (!area.levelNumber.has_value()) {
        if (requirement != nullptr && !requirement->levelNumbers.empty()) {
            errors.push_back({
                    .code = MapIssueCode::MissingAreaLevelNumber,
                    .areaId = area.id,
                    .areaType = *area.type,
            });
        }
    } else if (requirement == nullptr || requirement->levelNumbers.empty() ||
               std::find(requirement->levelNumbers.begin(), requirement->levelNumbers.end(),
                         *area.levelNumber) == requirement->levelNumbers.end()) {
        errors.push_back({
                .code = MapIssueCode::UnexpectedAreaLevelNumber,
                .areaId = area.id,
                .areaType = *area.type,
                .levelNumber = *area.levelNumber,
        });
    } else if (!levelNumbersByType[*area.type].insert(*area.levelNumber).second) {
        errors.push_back({
                .code = MapIssueCode::DuplicateLevelNumber,
                .areaId = area.id,
                .areaType = *area.type,
                .levelNumber = *area.levelNumber,
        });
    }
}

// Indexa las areas, revisa cada una y que el mundo tenga todas las requeridas.
AreaIndex validateAreas(const MapDefinition& map, const MapRequirements& requirements,
                        Issues& errors) {
    AreaIndex index;
    std::set<std::string> areaIds;
    std::map<AreaType, int> areaCounts;
    std::map<AreaType, std::set<int>> levelNumbersByType;

    for (const auto& area: map.areas) {
        if (area.id.empty()) {
            errors.push_back({.code = MapIssueCode::EmptyAreaId});
        } else if (!areaIds.insert(area.id).second) {
            errors.push_back({.code = MapIssueCode::DuplicateAreaId, .areaId = area.id});
        } else {
            index.byId.emplace(area.id, &area);
        }

        if (!area.type.has_value()) {
            errors.push_back({.code = MapIssueCode::MissingAreaType, .areaId = area.id});
        } else {
            ++areaCounts[*area.type];
            if (!area.id.empty()) {
                index.typeById.try_emplace(area.id, *area.type);
                index.levelById.try_emplace(area.id, area.levelNumber);
            }
        }

        // Comprueba que las dimensiones declaradas coincidan con la grilla de celdas.
        if (area.width <= 0 || area.height <= 0) {
            errors.push_back({.code = MapIssueCode::InvalidAreaDimensions, .areaId = area.id});
        }
        if (area.tiles.size() != static_cast<std::size_t>(area.height)) {
            errors.push_back({.code = MapIssueCode::TileGridHeightMismatch, .areaId = area.id});
        }
        const bool widthMismatch =
                std::any_of(area.tiles.begin(), area.tiles.end(), [&area](const auto& row) {
                    return row.size() != static_cast<std::size_t>(area.width);
                });
        if (widthMismatch) {
            errors.push_back({.code = MapIssueCode::TileGridWidthMismatch, .areaId = area.id});
        }

        if (area.type.has_value()) {
            validateLevelNumber(area, requirements, levelNumbersByType, errors);
        }
    }

    for (const auto& requirement: requirements.areas) {
        const auto countIt = areaCounts.find(requirement.type);
        const int actualCount = countIt == areaCounts.end() ? 0 : countIt->second;
        if (actualCount != requirement.count) {
            errors.push_back({
                    .code = MapIssueCode::AreaCountMismatch,
                    .areaType = requirement.type,
                    .expected = requirement.count,
                    .actual = actualCount,
            });
        }
        for (const int levelNumber: requirement.levelNumbers) {
            if (levelNumbersByType[requirement.type].find(levelNumber) ==
                levelNumbersByType[requirement.type].end()) {
                errors.push_back({
                        .code = MapIssueCode::MissingRequiredLevelNumber,
                        .areaType = requirement.type,
                        .levelNumber = levelNumber,
                });
            }
        }
    }
    return index;
}

// Verifica IDs, referencias, tamanos y posiciones de todos los elementos.
ElementIndex validateElements(const MapDefinition& map, const AreaIndex& areas, Issues& errors) {
    ElementIndex index;
    std::set<std::string> elementIds;
    for (const auto& element: map.elements) {
        if (element.id.empty()) {
            errors.push_back({.code = MapIssueCode::EmptyElementId});
        } else if (!elementIds.insert(element.id).second) {
            errors.push_back({.code = MapIssueCode::DuplicateElementId, .elementId = element.id});
        } else {
            index.byId.emplace(element.id, &element);
        }
        if (!element.type.has_value()) {
            errors.push_back({.code = MapIssueCode::MissingElementType, .elementId = element.id});
        }
        if (!element.layer.has_value()) {
            errors.push_back({.code = MapIssueCode::MissingElementLayer, .elementId = element.id});
        }

        const auto areaIt = areas.byId.find(element.areaId);
        if (areaIt == areas.byId.end()) {
            errors.push_back({
                    .code = MapIssueCode::UnknownElementArea,
                    .areaId = element.areaId,
                    .elementId = element.id,
            });
            continue;
        }

        const auto& area = *areaIt->second;
        if (element.width <= 0 || element.height <= 0) {
            errors.push_back({
                    .code = MapIssueCode::InvalidElementDimensions,
                    .areaId = element.areaId,
                    .elementId = element.id,
            });
            continue;
        }
        if (element.x < 0 || element.y < 0 || element.x >= area.width || element.y >= area.height ||
            element.width > area.width - element.x || element.height > area.height - element.y) {
            errors.push_back({
                    .code = MapIssueCode::ElementOutsideAreaBounds,
                    .areaId = element.areaId,
                    .elementId = element.id,
            });
            continue;
        }
        index.byArea[element.areaId].push_back(&element);
        index.valid.insert(&element);
    }
    return index;
}

// En el mapa general (editor y minimapa) cada area ocupa su lugar: no se pueden encimar.
void validateAreaLayout(const MapDefinition& map, Issues& errors) {
    for (std::size_t first = 0; first < map.areas.size(); ++first) {
        const MapArea& a = map.areas[first];
        for (std::size_t second = first + 1; second < map.areas.size(); ++second) {
            const MapArea& b = map.areas[second];
            const bool overlap =
                    a.offsetX < b.offsetX + b.width && b.offsetX < a.offsetX + a.width &&
                    a.offsetY < b.offsetY + b.height && b.offsetY < a.offsetY + a.height;
            if (overlap) {
                errors.push_back({
                        .code = MapIssueCode::AreasOverlap,
                        .areaId = a.id,
                        .relatedAreaId = b.id,
                });
            }
        }
    }
}

// Revisa el elemento que dispara la transicion: que exista, este en el area de origen y sirva
// para viajar.
bool validateTrigger(const MapTransition& transition, const ElementIndex& elements,
                     Issues& errors) {
    const auto triggerIt = elements.byId.find(transition.triggerElementId);
    if (triggerIt == elements.byId.end()) {
        errors.push_back({
                .code = MapIssueCode::UnknownTransitionTriggerElement,
                .elementId = transition.triggerElementId,
                .transitionId = transition.id,
        });
        return false;
    }
    bool valid = true;
    const auto& trigger = *triggerIt->second;
    if (trigger.areaId != transition.fromAreaId) {
        errors.push_back({
                .code = MapIssueCode::TriggerNotInSourceArea,
                .areaId = transition.fromAreaId,
                .elementId = trigger.id,
                .transitionId = transition.id,
        });
        valid = false;
    }
    const bool isTravelElement = trigger.type && elementTraits(*trigger.type).travel;
    if (!isTravelElement || trigger.layer != ElementLayer::Structure) {
        errors.push_back({
                .code = MapIssueCode::InvalidTransitionTrigger,
                .elementId = trigger.id,
                .transitionId = transition.id,
                .elementType = trigger.type,
                .expectedLayer = ElementLayer::Structure,
                .actualLayer = trigger.layer,
        });
        valid = false;
    }
    return valid && elements.valid.find(&trigger) != elements.valid.end();
}

// Valida que cada conexion una areas existentes mediante una puerta o escalera valida.
TransitionsByArea validateTransitions(const MapDefinition& map, const AreaIndex& areas,
                                      const ElementIndex& elements, Issues& errors) {
    TransitionsByArea byArea;
    std::set<std::string> transitionIds;
    for (const auto& transition: map.transitions) {
        bool validTransition = true;
        if (transition.id.empty()) {
            errors.push_back({.code = MapIssueCode::EmptyTransitionId});
            validTransition = false;
        } else if (!transitionIds.insert(transition.id).second) {
            errors.push_back({
                    .code = MapIssueCode::DuplicateTransitionId,
                    .transitionId = transition.id,
            });
            validTransition = false;
        }

        const auto sourceIt = areas.byId.find(transition.fromAreaId);
        const auto targetIt = areas.byId.find(transition.toAreaId);
        if (sourceIt == areas.byId.end()) {
            errors.push_back({
                    .code = MapIssueCode::UnknownTransitionSourceArea,
                    .areaId = transition.fromAreaId,
                    .transitionId = transition.id,
            });
            validTransition = false;
        }
        if (targetIt == areas.byId.end()) {
            errors.push_back({
                    .code = MapIssueCode::UnknownTransitionTargetArea,
                    .areaId = transition.toAreaId,
                    .transitionId = transition.id,
            });
            validTransition = false;
        }

        if (!validateTrigger(transition, elements, errors)) {
            validTransition = false;
        }

        if (targetIt != areas.byId.end()) {
            const auto& targetArea = *targetIt->second;
            if (transition.targetX < 0 || transition.targetY < 0 ||
                transition.targetX >= targetArea.width || transition.targetY >= targetArea.height) {
                errors.push_back({
                        .code = MapIssueCode::TransitionArrivalOutsideArea,
                        .areaId = transition.toAreaId,
                        .transitionId = transition.id,
                        .x = transition.targetX,
                        .y = transition.targetY,
                });
                validTransition = false;
            }
        }

        if (validTransition) {
            byArea[transition.fromAreaId].push_back(&transition);
        }
    }
    return byArea;
}

// Comprueba que todas las areas tengan una salida y cumplan sus destinos requeridos.
void validateRoutes(const MapRequirements& requirements, const AreaIndex& areas,
                    const TransitionsByArea& transitions, Issues& errors) {
    if (requirements.requireOutgoingTransitionForEachArea) {
        for (const auto& [areaId, areaType]: areas.typeById) {
            const auto outgoingIt = transitions.find(areaId);
            if (outgoingIt == transitions.end() || outgoingIt->second.empty()) {
                errors.push_back({
                        .code = MapIssueCode::AreaWithoutOutgoingTransition,
                        .areaId = areaId,
                });
            }
        }
    }
    for (const auto& requirement: requirements.transitions) {
        for (const auto& [sourceId, sourceType]: areas.typeById) {
            if (sourceType != requirement.fromAreaType ||
                (requirement.fromLevel.has_value() &&
                 areas.levelById.at(sourceId) != requirement.fromLevel)) {
                continue;
            }

            bool hasRequiredRoute = false;
            const auto outgoingIt = transitions.find(sourceId);
            if (outgoingIt != transitions.end()) {
                for (const auto* transition: outgoingIt->second) {
                    const auto targetTypeIt = areas.typeById.find(transition->toAreaId);
                    if (targetTypeIt == areas.typeById.end() ||
                        targetTypeIt->second != requirement.toAreaType) {
                        continue;
                    }
                    if (requirement.toLevel.has_value() &&
                        areas.levelById.at(transition->toAreaId) != requirement.toLevel) {
                        continue;
                    }
                    hasRequiredRoute = true;
                    break;
                }
            }
            if (!hasRequiredRoute) {
                errors.push_back({
                        .code = MapIssueCode::MissingRequiredTransition,
                        .areaId = sourceId,
                        .targetAreaType = requirement.toAreaType,
                        .targetLevel = requirement.toLevel,
                });
            }
        }
    }
}

// Evita que las huellas de dos estructuras se superpongan en una misma area.
void validateStructureOverlap(const ElementIndex& elements, Issues& errors) {
    for (const auto& [areaId, inArea]: elements.byArea) {
        for (std::size_t firstIndex = 0; firstIndex < inArea.size(); ++firstIndex) {
            const auto& first = *inArea[firstIndex];
            if (first.layer != ElementLayer::Structure) {
                continue;
            }
            for (std::size_t secondIndex = firstIndex + 1; secondIndex < inArea.size();
                 ++secondIndex) {
                const auto& second = *inArea[secondIndex];
                if (second.layer == ElementLayer::Structure && rectanglesOverlap(first, second)) {
                    errors.push_back({
                            .code = MapIssueCode::StructureOverlap,
                            .areaId = areaId,
                            .elementId = first.id,
                            .relatedElementId = second.id,
                    });
                }
            }
        }
    }
}

// La celda de enfrente de la puerta tiene que poder pisarse para entrar.
void validateDoors(const MapDefinition& map, const AreaIndex& areas, const ElementIndex& elements,
                   Issues& errors) {
    std::set<std::string> triggerIds;
    for (const auto& transition: map.transitions) {
        triggerIds.insert(transition.triggerElementId);
    }
    for (const auto& [areaId, inArea]: elements.byArea) {
        const MapArea& area = *areas.byId.at(areaId);
        for (const auto* building: inArea) {
            if (!building->type || !elementTraits(*building->type).hasDoor) {
                continue;
            }
            // Delante de cada celda de la puerta tiene que poder pararse alguien
            for (const MapCell& door: doorCells(*building)) {
                const int frontX = door.x;
                const int frontY = door.y + 1;
                const bool inside = frontY < area.height &&
                                    frontY < static_cast<int>(area.tiles.size()) &&
                                    frontX < static_cast<int>(area.tiles[frontY].size());
                const bool blocked =
                        !inside || !isWalkableTerrain(area.tiles[frontY][frontX]) ||
                        std::any_of(inArea.begin(), inArea.end(), [&](const MapElement* other) {
                            return blocksCell(*other, triggerIds, frontX, frontY);
                        });
                if (blocked) {
                    errors.push_back({
                            .code = MapIssueCode::BlockedDoor,
                            .areaId = areaId,
                            .elementId = building->id,
                            .x = frontX,
                            .y = frontY,
                    });
                }
            }
        }
    }
}

// El dibujo se estira al ancho del elemento y se apoya en su base, asi que lo alto sobresale hacia
// arriba (como el techo de una casa). La camara no sale del area: si se pasa del borde, se corta.
void validateSprites(const ElementIndex& elements, Issues& errors) {
    for (const MapElement* element: elements.valid) {
        if (!element->type) {
            continue;
        }
        const auto sprite = ELEMENT_SPRITES.find(*element->type);
        if (sprite == ELEMENT_SPRITES.end()) {
            continue;
        }
        const SpriteRect& rect = sprite->second.rect;
        const int drawnHeight = rect.height * element->width * TILE_SIZE / rect.width;
        const int top = (element->y + element->height) * TILE_SIZE - drawnHeight;
        if (top < 0) {
            errors.push_back({
                    .code = MapIssueCode::SpriteOutsideArea,
                    .areaId = element->areaId,
                    .elementId = element->id,
            });
        }
    }
}

const ElementRule* findRule(const MapRequirements& requirements, AreaType areaType,
                            ElementType type) {
    const auto rule =
            std::find_if(requirements.elements.begin(), requirements.elements.end(),
                         [areaType, type](const ElementRule& candidate) {
                             return candidate.areaType == areaType && candidate.type == type;
                         });
    return rule == requirements.elements.end() ? nullptr : &*rule;
}

// Cada elemento tiene que ir en un tipo de area que lo permita, medir lo que dice su regla y no
// pasarse de la cantidad maxima por area.
void validateElementRules(const MapRequirements& requirements, const AreaIndex& areas,
                          const ElementIndex& elements, Issues& errors) {
    for (const auto& [areaId, inArea]: elements.byArea) {
        const auto areaType = areas.typeById.find(areaId);
        if (areaType == areas.typeById.end()) {
            continue;  // el area sin tipo ya se reporto
        }
        std::map<ElementType, int> counts;
        for (const MapElement* element: inArea) {
            if (!element->type) {
                continue;
            }
            const ElementRule* rule = findRule(requirements, areaType->second, *element->type);
            if (rule == nullptr) {
                errors.push_back({
                        .code = MapIssueCode::ElementInWrongArea,
                        .areaId = areaId,
                        .elementId = element->id,
                        .areaType = areaType->second,
                        .elementType = element->type,
                });
                continue;
            }
            if (element->width != rule->width || element->height != rule->height) {
                errors.push_back({
                        .code = MapIssueCode::WrongElementSize,
                        .areaId = areaId,
                        .elementId = element->id,
                        .elementType = element->type,
                });
            }
            // Se reporta una vez, al pasarse
            if (rule->maxPerArea && ++counts[*element->type] == *rule->maxPerArea + 1) {
                errors.push_back({
                        .code = MapIssueCode::TooManyElements,
                        .areaId = areaId,
                        .elementType = element->type,
                        .expected = rule->maxPerArea,
                });
            }
        }
    }
}

// Comprueba las entidades requeridas segun la configuracion, sin listas hardcodeadas aca.
void validateRequiredElements(const MapDefinition& map, const MapRequirements& requirements,
                              const AreaIndex& areas, const ElementIndex& elements,
                              Issues& errors) {
    for (const auto& requirement: requirements.elements) {
        if (!requirement.required) {
            continue;
        }
        bool hasTargetArea = false;
        bool hasCorrectElement = false;
        const MapElement* wrongLayerElement = nullptr;
        for (const auto& [areaId, areaType]: areas.typeById) {
            if (areaType != requirement.areaType) {
                continue;
            }
            hasTargetArea = true;
            for (const auto& element: map.elements) {
                if (element.areaId != areaId || element.type != requirement.type) {
                    continue;
                }
                if (element.layer == requirement.layer &&
                    elements.valid.find(&element) != elements.valid.end()) {
                    hasCorrectElement = true;
                    break;
                }
                if (element.layer.has_value() && element.layer != requirement.layer) {
                    wrongLayerElement = &element;
                }
            }
        }
        if (!hasTargetArea || hasCorrectElement) {
            continue;
        }
        if (wrongLayerElement != nullptr) {
            errors.push_back({
                    .code = MapIssueCode::RequiredElementWrongLayer,
                    .areaId = wrongLayerElement->areaId,
                    .elementId = wrongLayerElement->id,
                    .areaType = requirement.areaType,
                    .elementType = requirement.type,
                    .expectedLayer = requirement.layer,
                    .actualLayer = wrongLayerElement->layer,
            });
        } else {
            errors.push_back({
                    .code = MapIssueCode::MissingRequiredElement,
                    .areaType = requirement.areaType,
                    .elementType = requirement.type,
                    .expectedLayer = requirement.layer,
            });
        }
    }
}

// Cada tipo de area acepta solo los tiles de su regla (los de los interiores, como el sleepable de
// las camas, no van en ninguna). Se informa una vez por tile no permitido en cada area: su primera
// casilla y cuantas hay.
void validateTiles(const MapDefinition& map, const MapRequirements& requirements, Issues& errors) {
    for (const MapArea& area: map.areas) {
        auto rule = std::find_if(
                requirements.tiles.begin(), requirements.tiles.end(),
                [&area](const TileRule& r) { return area.type && r.areaType == *area.type; });
        if (rule == requirements.tiles.end()) {
            continue;
        }
        std::map<TileType, MapValidationIssue> wrong;
        for (int y = 0; y < static_cast<int>(area.tiles.size()); ++y) {
            for (int x = 0; x < static_cast<int>(area.tiles[y].size()); ++x) {
                const TileType tile = area.tiles[y][x];
                if (std::find(rule->types.begin(), rule->types.end(), tile) != rule->types.end()) {
                    continue;
                }
                const MapValidationIssue firstCell{
                        .code = MapIssueCode::TileInWrongArea,
                        .areaId = area.id,
                        .areaType = area.type,
                        .actual = 0,
                        .x = x,
                        .y = y,
                };
                MapValidationIssue& issue = wrong.try_emplace(tile, firstCell).first->second;
                ++*issue.actual;
            }
        }
        for (const auto& [tile, issue]: wrong) {
            errors.push_back(issue);
        }
    }
}

}  // namespace

std::vector<MapValidationIssue> validateMap(const MapDefinition& map,
                                            const MapRequirements& requirements) {
    // Se acumulan errores para poder informar varios problemas en una sola validacion. Cada paso
    // deja indices que usan los siguientes.
    Issues errors;
    validateHeader(map, requirements, errors);
    const AreaIndex areas = validateAreas(map, requirements, errors);
    validateAreaLayout(map, errors);
    const ElementIndex elements = validateElements(map, areas, errors);
    const TransitionsByArea transitions = validateTransitions(map, areas, elements, errors);
    validateRoutes(requirements, areas, transitions, errors);
    if (!requirements.allowStructureOverlap) {
        validateStructureOverlap(elements, errors);
    }
    validateDoors(map, areas, elements, errors);
    validateSprites(elements, errors);
    validateElementRules(requirements, areas, elements, errors);
    validateRequiredElements(map, requirements, areas, elements, errors);
    validateTiles(map, requirements, errors);
    return errors;
}

}  // namespace stardew::maps
