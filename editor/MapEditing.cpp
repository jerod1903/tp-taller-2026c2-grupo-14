#include "editor/MapEditing.h"

#include <algorithm>
#include <optional>
#include <string>
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
    unsaved = false;
    if (mapChanged) {
        mapChanged();
    }
}

const maps::MapDefinition& MapEditing::map() const { return definition; }

bool MapEditing::hasUnsavedChanges() const { return unsaved; }

void MapEditing::markSaved() { unsaved = false; }

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
    unsaved = true;
    if (tileChanged) {
        tileChanged(cell);
    }
}

void MapEditing::togglePassage(const maps::MapCell& cell) {
    if (::togglePassage(definition, cell.x, cell.y)) {
        notifyMapChanged();
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
    notifyMapChanged();
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
    notifyMapChanged();
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
    notifyMapChanged();
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

bool MapEditing::placeArea(maps::AreaType type, const AreaSize& size,
                           const maps::MapCell& topLeft) {
    if (!areaPlacementOf(size, topLeft).fits) {
        return false;
    }
    addArea(definition, type, size, topLeft);
    notifyMapChanged();
    return true;
}

AreaMove MapEditing::areaPlacementOf(const AreaSize& size, const maps::MapCell& topLeft) const {
    const CellRect rect{topLeft.x, topLeft.y, size.width, size.height};
    return AreaMove{{rect}, !overlapsAreas(definition, {rect}, {}), 0};
}

void MapEditing::removeArea(const std::string& areaId) {
    const std::optional<AreaRemoval> removal = areaRemovalOf(areaId);
    if (!removal) {
        return;
    }
    const std::size_t entrancesBefore = dungeonEntrances(definition).size();
    removeAreas(definition, removal->areaIds);
    // Las salidas llevaban frente a una entrada que se fue con el area: pasan a la que quede
    const std::vector<const maps::MapElement*> entrances = dungeonEntrances(definition);
    if (entrances.size() < entrancesBefore && !entrances.empty()) {
        linkExits(definition, entrances.front()->id);
    }
    notifyMapChanged();
}

std::optional<AreaRemoval> MapEditing::areaRemovalOf(const std::string& areaId) const {
    const std::vector<std::string> ids = areasMovingWith(definition, areaId);
    if (ids.empty()) {
        return std::nullopt;
    }
    const auto elements = std::count_if(
            definition.elements.begin(), definition.elements.end(), [&ids](const auto& e) {
                return std::find(ids.begin(), ids.end(), e.areaId) != ids.end();
            });
    return AreaRemoval{ids, static_cast<int>(elements)};
}

bool MapEditing::placeElement(maps::ElementType type, const maps::MapCell& bottomLeft) {
    const maps::ElementRule* rule = ruleFor(requirements, type);
    if (!rule || !::placeElement(definition, *rule, bottomLeft)) {
        return false;
    }
    notifyMapChanged();
    return true;
}

bool MapEditing::placeEntrance(const maps::MapCell& bottomLeft,
                               const std::optional<AreaSize>& floorSize) {
    const maps::ElementRule* rule = ruleFor(requirements, maps::ElementType::DungeonEntrance);
    const std::optional<std::string> id =
            rule ? ::placeElement(definition, *rule, bottomLeft) : std::nullopt;
    if (!id) {
        return false;
    }
    if (!hasDungeonLevels(definition) && floorSize) {
        createMine(definition, requirements, *floorSize, *id);
    }
    linkEntrance(definition, *id);
    // Si no habia otra entrada (se borro con su area), las salidas pasan a llevar frente a esta
    if (dungeonEntrances(definition).size() == 1) {
        linkExits(definition, *id);
    }
    notifyMapChanged();
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

void MapEditing::notifyMapChanged() {
    unsaved = true;
    if (mapChanged) {
        mapChanged();
    }
}
