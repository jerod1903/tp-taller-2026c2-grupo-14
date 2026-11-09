#pragma once

#include <functional>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

// Cuanto ocuparia un elemento apoyado en una casilla y si entra ahi
struct Placement {
    int width;
    int height;
    bool fits;
};

// El mapa que se esta editando y las operaciones que lo cambian. No sabe nada de Qt: quien lo
// muestra se entera de los cambios con los callbacks.
class MapEditing {
public:
    // De las reglas solo se usan el tamano y la capa de los elementos que se colocan: las reglas se
    // revisan al guardar
    explicit MapEditing(const stardew::maps::MapRequirements& requirements);

    // Reemplaza el mapa (al abrir uno) y avisa que cambio todo
    void load(stardew::maps::MapDefinition map);
    const stardew::maps::MapDefinition& map() const;

    // Las operaciones reciben casillas del mapa general
    void paint(const stardew::maps::MapCell& cell, stardew::maps::TileType tile);
    void togglePassage(const stardew::maps::MapCell& cell);
    // Quita el elemento y sus transiciones. Un porton se va con su par; la entrada a la mina no se
    // borra.
    void removeElement(const std::string& id);
    // Pone un elemento de ese tipo con la esquina de abajo a la izquierda de su base en la casilla.
    // Devuelve false si la base no cae entera en una sola area o pisa otro elemento.
    bool placeElement(stardew::maps::ElementType type, const stardew::maps::MapCell& bottomLeft);
    // Lo que ocuparia un elemento de ese tipo apoyado en la casilla y si placeElement lo pondria.
    // Vacio si no hay regla para ese tipo.
    std::optional<Placement> placementOf(stardew::maps::ElementType type,
                                         const stardew::maps::MapCell& bottomLeft) const;

    // Cambio el terreno de una casilla
    void onTileChanged(std::function<void(const stardew::maps::MapCell&)> callback);
    // Cambio algo mas que una casilla (elementos, transiciones, el mapa entero)
    void onMapChanged(std::function<void()> callback);

private:
    const stardew::maps::MapRequirements& requirements;
    stardew::maps::MapDefinition definition;
    std::function<void(const stardew::maps::MapCell&)> tileChanged;
    std::function<void()> mapChanged;
};
