#pragma once

#include <string>

#include "common/map/MapDefinition.h"

namespace stardew::maps {

// Filas que ocupa la pared del fondo arriba del piso: WallTop, WallUpper, Wall y WallBase
inline constexpr int BACK_WALL_ROWS = 4;

// Arma las paredes de un interior a partir de su piso (las casillas que se pueden pisar): todo lo
// demas se reemplaza. Arriba del piso va la pared del fondo, a los costados las molduras, abajo el
// zocalo con sus esquinas y lo que sobra queda negro. Para que entre la pared del fondo, el piso
// tiene que empezar BACK_WALL_ROWS filas mas abajo que el borde del area. Dos cuartos se separan
// con una columna que no sea piso: ahi va una pared, y un hueco de piso en ella es la puerta.
void addWalls(MapArea& area);

// Lee un interior de una sola area (el mercado, el centro comunitario) y le arma las paredes.
// Lanza std::runtime_error si el archivo no se puede leer o no tiene exactamente un area
MapDefinition loadInterior(const std::string& path);

}  // namespace stardew::maps
