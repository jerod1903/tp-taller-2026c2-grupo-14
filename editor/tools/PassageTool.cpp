#include "editor/tools/PassageTool.h"

#include "editor/MapEditing.h"

PassageTool::PassageTool(MapEditing& editing): editing(editing) {}

void PassageTool::onPress(const stardew::maps::MapCell& cell) { editing.togglePassage(cell); }
