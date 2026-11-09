#include "editor/MapEditing.h"

#include <optional>
#include <utility>
#include <vector>

#include "editor/Elements.h"
#include "editor/MapHelper.h"
#include "editor/Passages.h"

namespace maps = stardew::maps;

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
    if (!element || element->type == maps::ElementType::DungeonEntrance) {
        return;
    }
    if (element->type == maps::ElementType::Door) {
        removePassage(definition, id);
    } else {
        ::removeElement(definition, id);
    }
    if (mapChanged) {
        mapChanged();
    }
}

void MapEditing::onTileChanged(std::function<void(const maps::MapCell&)> callback) {
    tileChanged = std::move(callback);
}

void MapEditing::onMapChanged(std::function<void()> callback) { mapChanged = std::move(callback); }
