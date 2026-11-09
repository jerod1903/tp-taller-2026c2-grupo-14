#include "editor/tools/SelectTool.h"

#include <utility>

#include "editor/ElementHelper.h"
#include "editor/MapEditing.h"
#include "editor/MapHelper.h"

namespace maps = stardew::maps;

SelectTool::SelectTool(MapEditing& editing): editing(editing) {}

void SelectTool::onPress(const maps::MapCell& cell) {
    const std::optional<AreaCell> target = cellAt(editing.map(), cell.x, cell.y);
    const maps::MapElement* element = target ? elementAt(editing.map(), *target) : nullptr;
    select(element ? std::optional<std::string>(element->id) : std::nullopt);
    drag.reset();
    if (element) {
        // Se mueve manteniendo la casilla de la base por donde se lo agarro bajo el mouse
        const maps::MapCell bottomLeft = bottomLeftOf(editing.map(), *element);
        drag = Drag{element->id, cell.x - bottomLeft.x, cell.y - bottomLeft.y, std::nullopt};
    }
}

void SelectTool::onMove(const maps::MapCell& cell) {
    if (!drag) {
        return;
    }
    const maps::MapCell bottomLeft{cell.x - drag->grabX, cell.y - drag->grabY};
    if (drag->bottomLeft && drag->bottomLeft->x == bottomLeft.x &&
        drag->bottomLeft->y == bottomLeft.y) {
        return;
    }
    const std::optional<Placement> movement = editing.movementOf(drag->id, bottomLeft);
    const maps::MapElement* element = elementById(editing.map(), drag->id);
    if (!movement || !element || !element->type) {
        return;
    }
    drag->bottomLeft = bottomLeft;
    showGhost(Ghost{*element->type, bottomLeft, movement->width, movement->height, movement->fits});
}

void SelectTool::onRelease(const maps::MapCell&) {
    if (!drag) {
        return;
    }
    const Drag dropped = *drag;
    drag.reset();
    showGhost(std::nullopt);
    // Sin arrastrar fue solo un click; si no entra, se queda donde estaba
    if (dropped.bottomLeft) {
        editing.moveElement(dropped.id, *dropped.bottomLeft);
    }
}

void SelectTool::clear() {
    if (drag) {
        drag.reset();
        showGhost(std::nullopt);
    }
    select(std::nullopt);
}

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

void SelectTool::onGhostChanged(std::function<void(const std::optional<Ghost>&)> callback) {
    ghostChanged = std::move(callback);
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

void SelectTool::showGhost(const std::optional<Ghost>& ghost) {
    if (ghostChanged) {
        ghostChanged(ghost);
    }
}
