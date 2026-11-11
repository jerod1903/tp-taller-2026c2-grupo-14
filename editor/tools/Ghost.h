#pragma once

#include <vector>

#include "common/map/MapDefinition.h"
#include "editor/MapHelper.h"

// Lo que se muestra bajo el mouse antes de colocar o mover un elemento: el elemento apoyado con la
// esquina de abajo a la izquierda de su base en la casilla, y si entra ahi
struct Ghost {
    stardew::maps::ElementType type;
    stardew::maps::MapCell bottomLeft;
    int width;
    int height;
    bool fits;
};

// Donde quedarian las areas que se estan moviendo (los pisos de la mina se mueven juntos) y si
// entran ahi sin pisar otra
struct AreaGhost {
    std::vector<CellRect> areas;
    bool fits;
};
