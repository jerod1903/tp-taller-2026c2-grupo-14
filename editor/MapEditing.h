#pragma once

#include <functional>

#include "common/map/MapDefinition.h"

// El mapa que se esta editando y las operaciones que lo cambian. No sabe nada de Qt: quien lo
// muestra se entera de los cambios con los callbacks.
class MapEditing {
public:
    // Reemplaza el mapa (al abrir uno) y avisa que cambio todo
    void load(stardew::maps::MapDefinition map);
    const stardew::maps::MapDefinition& map() const;

    // Las operaciones reciben casillas del mapa general
    void paint(const stardew::maps::MapCell& cell, stardew::maps::TileType tile);
    void togglePassage(const stardew::maps::MapCell& cell);

    // Cambio el terreno de una casilla
    void onTileChanged(std::function<void(const stardew::maps::MapCell&)> callback);
    // Cambio algo mas que una casilla (elementos, transiciones, el mapa entero)
    void onMapChanged(std::function<void()> callback);

private:
    stardew::maps::MapDefinition definition;
    std::function<void(const stardew::maps::MapCell&)> tileChanged;
    std::function<void()> mapChanged;
};
