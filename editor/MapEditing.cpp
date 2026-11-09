#include "editor/MapEditing.h"

#include <optional>
#include <utility>
#include <vector>

#include "editor/Areas.h"
#include "editor/ElementHelper.h"
#include "editor/Elements.h"
#include "editor/MapHelper.h"
#include "editor/MineHelper.h"
#include "editor/Passages.h"

namespace maps = stardew::maps;

namespace {
// La salida y la escalera de un piso: estan en todos los pisos en la misma casilla
bool isFloorElement(const maps::MapElement& element) {
    return element.type == maps::ElementType::Exit || element.type == maps::ElementType::Stairs;
}
}  // namespace

MapEditing::MapEditing(const maps::MapRequirements& requirements): requirements(requirements) {}

void MapEditing::load(maps::MapDefinition map) {
    definition = std::move(map);
    if (mapChanged) {
        mapChanged();
    }
}

const maps::MapDefinition& MapEditing::map() const { return definition; }

void MapEditing::paint(const maps::MapCell& cell, maps::TileType tile) {
    const std::optional<AreaCell> target = cellAt(definition, cell.x, cell.y);
    if (!target) {
        return;
    }
    std::vector<std::vector<maps::TileType>>& tiles = definition.areas[target->area].tiles;
    // Una grilla mas chica que lo declarado (mapa con errores) no se pinta fuera de rango
    if (target->y >= static_cast<int>(tiles.size()) ||
        target->x >= static_cast<int>(tiles[target->y].size()) ||
        tiles[target->y][target->x] == tile) {
        return;
    }
    tiles[target->y][target->x] = tile;
    if (tileChanged) {
        tileChanged(cell);
    }
}

void MapEditing::togglePassage(const maps::MapCell& cell) {
    if (::togglePassage(definition, cell.x, cell.y) && mapChanged) {
        mapChanged();
    }
}

void MapEditing::removeElement(const std::string& id) {
    const maps::MapElement* element = elementById(definition, id);
    if (!element || isFloorElement(*element)) {
        return;
    }
    const bool entrance = element->type == maps::ElementType::DungeonEntrance;
    if (element->type == maps::ElementType::Door) {
        removePassage(definition, id);
    } else {
        ::removeElement(definition, id);
    }
    // Las salidas llevaban frente a la entrada borrada: pasan a la que quede
    const std::vector<const maps::MapElement*> entrances = dungeonEntrances(definition);
    if (entrance && !entrances.empty()) {
        linkExits(definition, entrances.front()->id);
    }
    if (mapChanged) {
        mapChanged();
    }
}

bool MapEditing::moveElement(const std::string& id, const maps::MapCell& bottomLeft) {
    const maps::MapElement* element = elementById(definition, id);
    if (!element || element->type == maps::ElementType::Door) {
        return false;
    }
    const std::optional<maps::ElementType> type = element->type;
    const bool moved = isFloorElement(*element) ? moveInEveryFloor(definition, *type, bottomLeft) :
                                                  ::moveElement(definition, id, bottomLeft);
    if (!moved) {
        return false;
    }
    // Las salidas llevan frente a la entrada: si se movio una, la siguen
    if (type == maps::ElementType::DungeonEntrance) {
        linkExits(definition, id);
    }
    if (mapChanged) {
        mapChanged();
    }
    return true;
}

std::optional<Placement> MapEditing::movementOf(const std::string& id,
                                                const maps::MapCell& bottomLeft) const {
    const maps::MapElement* element = elementById(definition, id);
    if (!element || element->type == maps::ElementType::Door) {
        return std::nullopt;
    }
    const bool fits =
            isFloorElement(*element) ?
                    fitsInEveryFloor(definition, *element->type, bottomLeft) :
                    freeBaseAt(definition, bottomLeft, element->width, element->height, id)
                            .has_value();
    return Placement{element->width, element->height, fits};
}

bool MapEditing::moveArea(const std::string& areaId, int dx, int dy) {
    const std::optional<AreaMove> move = areaMoveOf(areaId, dx, dy);
    if (!move || !move->fits) {
        return false;
    }
    moveAreas(definition, areasMovingWith(definition, areaId), dx, dy);
    if (mapChanged) {
        mapChanged();
    }
    return true;
}

std::optional<AreaMove> MapEditing::areaMoveOf(const std::string& areaId, int dx, int dy) const {
    const std::vector<std::string> ids = areasMovingWith(definition, areaId);
    if (ids.empty()) {
        return std::nullopt;
    }
    AreaMove move{{}, true, 0};
    for (const std::string& id: ids) {
        CellRect rect = areaRect(*areaById(definition, id));
        rect.x += dx;
        rect.y += dy;
        move.areas.push_back(rect);
    }
    move.fits = !overlapsAreas(definition, move.areas, ids);
    move.brokenPassages = static_cast<int>(passagesBrokenBy(definition, ids, dx, dy).size());
    return move;
}

bool MapEditing::placeElement(maps::ElementType type, const maps::MapCell& bottomLeft) {
    const maps::ElementRule* rule = ruleFor(requirements, type);
    if (!rule || !::placeElement(definition, *rule, bottomLeft)) {
        return false;
    }
    if (mapChanged) {
        mapChanged();
    }
    return true;
}

bool MapEditing::placeEntrance(const maps::MapCell& bottomLeft,
                               const std::optional<FloorSize>& floorSize) {
    const maps::ElementRule* rule = ruleFor(requirements, maps::ElementType::DungeonEntrance);
    const std::optional<std::string> id =
            rule ? ::placeElement(definition, *rule, bottomLeft) : std::nullopt;
    if (!id) {
        return false;
    }
    if (!hasDungeonFloors(definition) && floorSize) {
        createMine(definition, requirements, *floorSize, *id);
    }
    linkEntrance(definition, *id);
    if (mapChanged) {
        mapChanged();
    }
    return true;
}

std::optional<Placement> MapEditing::placementOf(maps::ElementType type,
                                                 const maps::MapCell& bottomLeft) const {
    const maps::ElementRule* rule = ruleFor(requirements, type);
    if (!rule) {
        return std::nullopt;
    }
    const bool fits = freeBaseAt(definition, bottomLeft, rule->width, rule->height).has_value();
    return Placement{rule->width, rule->height, fits};
}

void MapEditing::onTileChanged(std::function<void(const maps::MapCell&)> callback) {
    tileChanged = std::move(callback);
}

void MapEditing::onMapChanged(std::function<void()> callback) { mapChanged = std::move(callback); }
