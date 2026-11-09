#pragma once

#include <functional>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "editor/tools/Tool.h"

class MapEditing;

// Un click sobre la base de un elemento lo selecciona; en una casilla libre, se deja de
// seleccionar. Guarda el id del elemento y avisa cada vez que cambia lo seleccionado.
class SelectTool: public Tool {
public:
    explicit SelectTool(MapEditing& editing);
    void onPress(const stardew::maps::MapCell& cell) override;
    // Deja de haber algo seleccionado
    void clear();
    // Borra el elemento seleccionado. Si no se pudo borrar (la entrada a la mina) sigue
    // seleccionado.
    void removeSelected();
    // Cambio lo seleccionado: el id del elemento, o vacio si no hay ninguno
    void onSelectionChanged(std::function<void(const std::optional<std::string>&)> callback);

private:
    void select(std::optional<std::string> id);

    MapEditing& editing;
    std::optional<std::string> selected;
    std::function<void(const std::optional<std::string>&)> selectionChanged;
};
