#include "editor/tools/PaintTool.h"

#include "editor/MapEditing.h"

namespace maps = stardew::maps;

PaintTool::PaintTool(MapEditing& editing): editing(editing) {}

void PaintTool::setTile(maps::TileType chosen) { tile = chosen; }

void PaintTool::onPress(const maps::MapCell& cell) { editing.paint(cell, tile); }

void PaintTool::onMove(const maps::MapCell& cell) { editing.paint(cell, tile); }
