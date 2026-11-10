#include "editor/tools/EntranceTool.h"

#include "editor/MapEditing.h"

namespace maps = stardew::maps;

EntranceTool::EntranceTool(MapEditing& editing): PlaceTool(editing) {
    setType(maps::ElementType::DungeonEntrance);
}

void EntranceTool::setFloorSize(std::optional<AreaSize> size) { floorSize = size; }

bool EntranceTool::place(maps::ElementType, const maps::MapCell& cell) {
    return editing.placeEntrance(cell, floorSize);
}
