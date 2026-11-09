#include "editor/tools/SelectTool.h"

#include <utility>

#include "editor/MapEditing.h"
#include "editor/MapHelper.h"

namespace maps = stardew::maps;

SelectTool::SelectTool(MapEditing& editing): editing(editing) {}

void SelectTool::onPress(const maps::MapCell& cell) {
    const std::optional<AreaCell> target = cellAt(editing.map(), cell.x, cell.y);
    const maps::MapElement* element = target ? elementAt(editing.map(), *target) : nullptr;
    select(element ? std::optional<std::string>(element->id) : std::nullopt);
}

void SelectTool::clear() { select(std::nullopt); }

void SelectTool::removeSelected() {
    if (!selected) {
        return;
    }
    editing.removeElement(*selected);
    if (!elementById(editing.map(), *selected)) {
        select(std::nullopt);
    }
}

void SelectTool::onSelectionChanged(
        std::function<void(const std::optional<std::string>&)> callback) {
    selectionChanged = std::move(callback);
}

void SelectTool::select(std::optional<std::string> id) {
    if (id == selected) {
        return;
    }
    selected = std::move(id);
    if (selectionChanged) {
        selectionChanged(selected);
    }
}
