#include "editor/tools/AreaTool.h"

#include <utility>

#include "editor/MapEditing.h"
#include "editor/MapHelper.h"

namespace maps = stardew::maps;

AreaTool::AreaTool(MapEditing& editing): editing(editing) {}

void AreaTool::onPress(const maps::MapCell& cell) {
    const std::optional<AreaCell> target = cellAt(editing.map(), cell.x, cell.y);
    drag.reset();
    if (!target) {
        select(std::nullopt);
        return;
    }
    const std::string areaId = editing.map().areas[target->area].id;
    select(areaId);
    drag = Drag{areaId, cell, 0, 0};
}

void AreaTool::onMove(const maps::MapCell& cell) {
    if (!drag) {
        return;
    }
    const int dx = cell.x - drag->start.x;
    const int dy = cell.y - drag->start.y;
    if (dx == drag->dx && dy == drag->dy) {
        return;
    }
    drag->dx = dx;
    drag->dy = dy;
    const std::optional<AreaMove> move = editing.areaMoveOf(drag->areaId, dx, dy);
    showGhost(move ? std::optional<AreaGhost>(AreaGhost{move->areas, move->fits}) : std::nullopt);
}

void AreaTool::onRelease(const maps::MapCell&) {
    if (!drag) {
        return;
    }
    const Drag dropped = *drag;
    stop();
    if (dropped.dx == 0 && dropped.dy == 0) {
        return;
    }
    const std::optional<AreaMove> move = editing.areaMoveOf(dropped.areaId, dropped.dx, dropped.dy);
    if (!move || !move->fits) {
        return;
    }
    if (move->brokenPassages > 0 && !confirmed(move->brokenPassages)) {
        return;
    }
    editing.moveArea(dropped.areaId, dropped.dx, dropped.dy);
}

void AreaTool::onCancel() { stop(); }

void AreaTool::clear() {
    if (drag) {
        stop();
    }
    select(std::nullopt);
}

const std::optional<std::string>& AreaTool::selected() const { return selectedArea; }

void AreaTool::removeSelected() {
    if (!selectedArea) {
        return;
    }
    editing.removeArea(*selectedArea);
    select(std::nullopt);
}

void AreaTool::onSelectionChanged(std::function<void(const std::optional<std::string>&)> callback) {
    selectionChanged = std::move(callback);
}

void AreaTool::select(std::optional<std::string> areaId) {
    if (areaId == selectedArea) {
        return;
    }
    selectedArea = std::move(areaId);
    if (selectionChanged) {
        selectionChanged(selectedArea);
    }
}

void AreaTool::onGhostChanged(std::function<void(const std::optional<AreaGhost>&)> callback) {
    ghostChanged = std::move(callback);
}

void AreaTool::onConfirmBrokenPassages(std::function<bool(int)> callback) {
    confirmBrokenPassages = std::move(callback);
}

bool AreaTool::confirmed(int brokenPassages) const {
    return !confirmBrokenPassages || confirmBrokenPassages(brokenPassages);
}

void AreaTool::showGhost(const std::optional<AreaGhost>& ghost) {
    if (ghostChanged) {
        ghostChanged(ghost);
    }
}

void AreaTool::stop() {
    drag.reset();
    showGhost(std::nullopt);
}
