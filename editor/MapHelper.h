#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"

// Consultas al mapa que comparten el editor y sus herramientas. Ninguna cambia el mapa.

// Una casilla del mapa general traducida a su area: el indice del area en map.areas y la casilla
// dentro de ella
struct AreaCell {
    std::size_t area;
    int x;
    int y;
};

// La casilla (x, y) del mapa general, con los offsets de las areas, dentro de su area. Vacio si no
// cae en ninguna.
std::optional<AreaCell> cellAt(const stardew::maps::MapDefinition& map, int x, int y);

// El area con ese id, o nullptr si no hay
const stardew::maps::MapArea* areaById(const stardew::maps::MapDefinition& map,
                                       const std::string& id);

// El elemento que ocupa la casilla, o nullptr si esta libre
const stardew::maps::MapElement* elementAt(const stardew::maps::MapDefinition& map,
                                           const AreaCell& cell);

// El elemento con ese id, o nullptr si no hay
const stardew::maps::MapElement* elementById(const stardew::maps::MapDefinition& map,
                                             const std::string& id);

// Si el terreno de la casilla se puede pisar
bool walkable(const stardew::maps::MapDefinition& map, const AreaCell& cell);

// La casilla corrida (dx, dy) dentro de la misma area, o vacio si se sale
std::optional<AreaCell> inside(const stardew::maps::MapDefinition& map, const AreaCell& cell,
                               int dx, int dy);

// Si algun elemento o transicion ya usa ese id
bool idTaken(const stardew::maps::MapDefinition& map, const std::string& id);
