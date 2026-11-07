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
    std::map<std::string, std::optional<int>> floorById;
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
void validateFloorNumber(const MapArea& area, const MapRequirements& requirements,
                         std::map<AreaType, std::set<int>>& floorNumbersByType, Issues& errors) {
    const auto* requirement = findAreaRequirement(requirements, *area.type);
    if (!area.floorNumber.has_value()) {
        if (requirement != nullptr && !requirement->floorNumbers.empty()) {
            errors.push_back({
                    .code = MapIssueCode::MissingAreaFloorNumber,
                    .areaId = area.id,
                    .areaType = *area.type,
            });
        }
    } else if (requirement == nullptr || requirement->floorNumbers.empty() ||
               std::find(requirement->floorNumbers.begin(), requirement->floorNumbers.end(),
                         *area.floorNumber) == requirement->floorNumbers.end()) {
        errors.push_back({
                .code = MapIssueCode::UnexpectedAreaFloorNumber,
                .areaId = area.id,
                .areaType = *area.type,
                .floorNumber = *area.floorNumber,
        });
    } else if (!floorNumbersByType[*area.type].insert(*area.floorNumber).second) {
        errors.push_back({
                .code = MapIssueCode::DuplicateFloorNumber,
                .areaId = area.id,
                .areaType = *area.type,
                .floorNumber = *area.floorNumber,
        });
    }
}

// Indexa las areas, revisa cada una y que el mundo tenga todas las requeridas.
AreaIndex validateAreas(const MapDefinition& map, const MapRequirements& requirements,
                        Issues& errors) {
    AreaIndex index;
    std::set<std::string> areaIds;
    std::map<AreaType, int> areaCounts;
    std::map<AreaType, std::set<int>> floorNumbersByType;

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
                index.floorById.try_emplace(area.id, area.floorNumber);
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
            validateFloorNumber(area, requirements, floorNumbersByType, errors);
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
        for (const int floorNumber: requirement.floorNumbers) {
            if (floorNumbersByType[requirement.type].find(floorNumber) ==
                floorNumbersByType[requirement.type].end()) {
                errors.push_back({
                        .code = MapIssueCode::MissingRequiredFloorNumber,
                        .areaType = requirement.type,
                        .floorNumber = floorNumber,
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
    const bool isTravelElement = trigger.type == ElementType::Door ||
                                 trigger.type == ElementType::Stairs ||
                                 trigger.type == ElementType::DungeonEntrance;
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
                (requirement.fromFloor.has_value() &&
                 areas.floorById.at(sourceId) != requirement.fromFloor)) {
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
                    if (requirement.toFloor.has_value() &&
                        areas.floorById.at(transition->toAreaId) != requirement.toFloor) {
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
                        .targetFloor = requirement.toFloor,
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
            if (!building->type || !hasDoor(*building->type)) {
                continue;
            }
            const MapCell door = doorCell(*building);
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

// Comprueba las entidades requeridas segun la configuracion, sin listas hardcodeadas aca.
void validateRequiredElements(const MapDefinition& map, const MapRequirements& requirements,
                              const AreaIndex& areas, const ElementIndex& elements,
                              Issues& errors) {
    for (const auto& requirement: requirements.elements) {
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

}  // namespace

std::vector<MapValidationIssue> validateMap(const MapDefinition& map,
                                            const MapRequirements& requirements) {
    // Se acumulan errores para poder informar varios problemas en una sola validacion. Cada paso
    // deja indices que usan los siguientes.
    Issues errors;
    validateHeader(map, requirements, errors);
    const AreaIndex areas = validateAreas(map, requirements, errors);
    const ElementIndex elements = validateElements(map, areas, errors);
    const TransitionsByArea transitions = validateTransitions(map, areas, elements, errors);
    validateRoutes(requirements, areas, transitions, errors);
    if (!requirements.allowStructureOverlap) {
        validateStructureOverlap(elements, errors);
    }
    validateDoors(map, areas, elements, errors);
    validateSprites(elements, errors);
    validateRequiredElements(map, requirements, areas, elements, errors);
    return errors;
}

}  // namespace stardew::maps
