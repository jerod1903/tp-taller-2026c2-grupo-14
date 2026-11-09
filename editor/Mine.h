#pragma once

#include <string>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

// El tamano de cada piso de la mina, en casillas. Todos los pisos miden lo mismo.
struct FloorSize {
    int width;
    int height;
};

// Crea los pisos que piden las reglas (mina-1, mina-2...), debajo de las areas que ya hay y en
// filas de cinco. Cada piso tiene pared en el borde y piso adentro, la salida arriba a la
// izquierda y la escalera abajo a la derecha (el ultimo piso no tiene). La escalera lleva al piso
// siguiente, al lado de su salida, y la salida lleva frente a la entrada.
void createMine(stardew::maps::MapDefinition& map,
                const stardew::maps::MapRequirements& requirements, const FloorSize& size,
                const std::string& entranceId);

// La transicion de la entrada al primer piso, al lado de su salida. No hace nada si no hay pisos.
void linkEntrance(stardew::maps::MapDefinition& map, const std::string& entranceId);
