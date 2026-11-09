#pragma once

#include "common/map/MapDefinition.h"

// Lo que hace el click izquierdo sobre el lienzo. El lienzo convierte el mouse en una casilla del
// mapa general (con los offsets de las areas) y se la pasa a la herramienta activa, sin saber cual
// es. Mover y soltar no hacen nada salvo que la herramienta los necesite.
class Tool {
public:
    virtual ~Tool() = default;
    virtual void onPress(const stardew::maps::MapCell& cell) = 0;
    virtual void onMove(const stardew::maps::MapCell&) {}
    virtual void onRelease(const stardew::maps::MapCell&) {}
};
