#include "editor/tools/MoveAreaTool.h"

#include <utility>

#include "editor/MapEditing.h"
#include "editor/MapHelper.h"

namespace maps = stardew::maps;

MoveAreaTool::MoveAreaTool(MapEditing& editing): editing(editing) {}

void MoveAreaTool::onPress(const maps::MapCell& cell) {
    const std::optional<AreaCell> target = cellAt(editing.map(), cell.x, cell.y);
    drag.reset();
    if (target) {
        drag = Drag{editing.map().areas[target->area].id, cell, 0, 0};
    }
}

void MoveAreaTool::onMove(const maps::MapCell& cell) {
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

void MoveAreaTool::onRelease(const maps::MapCell&) {
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

void MoveAreaTool::onCancel() { stop(); }

void MoveAreaTool::onGhostChanged(std::function<void(const std::optional<AreaGhost>&)> callback) {
    ghostChanged = std::move(callback);
}

void MoveAreaTool::onConfirmBrokenPassages(std::function<bool(int)> callback) {
    confirmBrokenPassages = std::move(callback);
}

bool MoveAreaTool::confirmed(int brokenPassages) const {
    return !confirmBrokenPassages || confirmBrokenPassages(brokenPassages);
}

void MoveAreaTool::showGhost(const std::optional<AreaGhost>& ghost) {
    if (ghostChanged) {
        ghostChanged(ghost);
    }
}

void MoveAreaTool::stop() {
    drag.reset();
    showGhost(std::nullopt);
}
