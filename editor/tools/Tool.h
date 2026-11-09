#pragma once

#include "common/map/MapDefinition.h"

// Lo que hace el mouse sobre el lienzo. El lienzo convierte el mouse en una casilla del mapa
// general (con los offsets de las areas) y se la pasa a la herramienta activa, sin saber cual es.
// Solo el click izquierdo es obligatorio: mover (con o sin boton), soltar y cancelar (click
// derecho) no hacen nada salvo que la herramienta los necesite.
class Tool {
public:
    virtual ~Tool() = default;
    virtual void onPress(const stardew::maps::MapCell& cell) = 0;
    // El mouse se movio con el click izquierdo apretado
    virtual void onMove(const stardew::maps::MapCell&) {}
    virtual void onRelease(const stardew::maps::MapCell&) {}
    // El mouse se movio sin ningun boton apretado
    virtual void onHover(const stardew::maps::MapCell&) {}
    virtual void onCancel() {}
};
