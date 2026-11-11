#include "editor/tools/PlaceAreaTool.h"

#include <utility>

#include "editor/MapEditing.h"

namespace maps = stardew::maps;

PlaceAreaTool::PlaceAreaTool(MapEditing& editing): editing(editing) {}

void PlaceAreaTool::setArea(maps::AreaType type, const AreaSize& size) {
    area = NewArea{type, size};
}

void PlaceAreaTool::onPress(const maps::MapCell& cell) {
    if (area && editing.placeArea(area->type, area->size, cell)) {
        finish();
    }
}

void PlaceAreaTool::onHover(const maps::MapCell& cell) {
    if (!area) {
        return;
    }
    const AreaMove placement = editing.areaPlacementOf(area->size, cell);
    showGhost(AreaGhost{placement.areas, placement.fits});
}

void PlaceAreaTool::onCancel() { finish(); }

void PlaceAreaTool::onGhostChanged(std::function<void(const std::optional<AreaGhost>&)> callback) {
    ghostChanged = std::move(callback);
}

void PlaceAreaTool::onDone(std::function<void()> callback) { done = std::move(callback); }

void PlaceAreaTool::showGhost(const std::optional<AreaGhost>& ghost) {
    if (ghostChanged) {
        ghostChanged(ghost);
    }
}

void PlaceAreaTool::finish() {
    showGhost(std::nullopt);
    if (done) {
        done();
    }
}
