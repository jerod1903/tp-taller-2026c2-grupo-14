#pragma once

#include <functional>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "editor/tools/Ghost.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Un click sobre la base de un elemento lo selecciona; en una casilla libre, se deja de
// seleccionar. Guarda el id del elemento y avisa cada vez que cambia lo seleccionado. Arrastrar el
// elemento lo mueve: mientras se arrastra avisa el fantasma y al soltar lo lleva ahi, si entra.
class SelectTool: public Tool {
public:
    explicit SelectTool(MapEditing& editing);
    void onPress(const stardew::maps::MapCell& cell) override;
    void onMove(const stardew::maps::MapCell& cell) override;
    void onRelease(const stardew::maps::MapCell& cell) override;
    // Deja de haber algo seleccionado
    void clear();
    // Borra el elemento seleccionado. Si no se pudo borrar (la salida o la escalera de un piso)
    // sigue seleccionado.
    void removeSelected();
    // Cambio lo seleccionado: el id del elemento, o vacio si no hay ninguno
    void onSelectionChanged(std::function<void(const std::optional<std::string>&)> callback);
    // Cambio el fantasma del elemento que se arrastra: vacio si no hay que mostrarlo
    void onGhostChanged(std::function<void(const std::optional<Ghost>&)> callback);

private:
    // El elemento que se esta arrastrando: desde que casilla de su base se lo agarro y donde
    // quedaria
    struct Drag {
        std::string id;
        int grabX;
        int grabY;
        std::optional<stardew::maps::MapCell> bottomLeft;
    };

    void select(std::optional<std::string> id);
    void showGhost(const std::optional<Ghost>& ghost);

    MapEditing& editing;
    std::optional<std::string> selected;
    std::optional<Drag> drag;
    std::function<void(const std::optional<std::string>&)> selectionChanged;
    std::function<void(const std::optional<Ghost>&)> ghostChanged;
};
