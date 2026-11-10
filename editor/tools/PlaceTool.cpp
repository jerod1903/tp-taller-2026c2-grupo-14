#include "editor/tools/PlaceTool.h"

#include <utility>

#include "editor/MapEditing.h"
#include "editor/MapHelper.h"

namespace maps = stardew::maps;

PlaceTool::PlaceTool(MapEditing& editing): editing(editing) {}

void PlaceTool::setType(maps::ElementType elementType) { type = elementType; }

void PlaceTool::onPress(const maps::MapCell& cell) {
    if (type && place(*type, cell)) {
        finish();
    }
}

bool PlaceTool::place(maps::ElementType elementType, const maps::MapCell& cell) {
    return editing.placeElement(elementType, cell);
}

void PlaceTool::onHover(const maps::MapCell& cell) {
    if (!type || !cellAt(editing.map(), cell.x, cell.y)) {
        showGhost(std::nullopt);
        return;
    }
    const std::optional<Placement> placement = editing.placementOf(*type, cell);
    if (!placement) {
        showGhost(std::nullopt);
        return;
    }
    showGhost(Ghost{*type, cell, placement->width, placement->height, placement->fits});
}

void PlaceTool::onCancel() { finish(); }

void PlaceTool::onGhostChanged(std::function<void(const std::optional<Ghost>&)> callback) {
    ghostChanged = std::move(callback);
}

void PlaceTool::onDone(std::function<void()> callback) { done = std::move(callback); }

void PlaceTool::showGhost(const std::optional<Ghost>& ghost) {
    if (ghostChanged) {
        ghostChanged(ghost);
    }
}

void PlaceTool::finish() {
    showGhost(std::nullopt);
    if (done) {
        done();
    }
}
