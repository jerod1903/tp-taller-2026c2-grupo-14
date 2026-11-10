#pragma once

#include <array>
#include <cstddef>
#include <optional>
#include <string>
#include <vector>

#include "common/map/MapDefinition.h"

// Consultas sobre las casillas y las areas del mapa que comparten el editor y sus herramientas.
// Ninguna cambia el mapa. Las de elementos estan en ElementHelper y las de la mina en MineHelper.

// Las casillas que se dejan entre las areas cuando el editor las acomoda
constexpr int AREA_GAP = 2;

// Una casilla del mapa general traducida a su area: el indice del area en map.areas y la casilla
// dentro de ella
struct AreaCell {
    std::size_t area;
    int x;
    int y;
};

// El tamano de un area, en casillas
struct AreaSize {
    int width;
    int height;
};

// Un rectangulo de casillas del mapa general
struct CellRect {
    int x;
    int y;
    int width;
    int height;
};

// Un paso hacia una casilla vecina
struct Step {
    int dx;
    int dy;
};

// Las cuatro casillas vecinas, en este orden: derecha, izquierda, abajo y arriba
constexpr std::array<Step, 4> NEIGHBOURS = {{{1, 0}, {-1, 0}, {0, 1}, {0, -1}}};

// La casilla (x, y) del mapa general, con los offsets de las areas, dentro de su area. Vacio si no
// cae en ninguna.
std::optional<AreaCell> cellAt(const stardew::maps::MapDefinition& map, int x, int y);

// El area con ese id, o nullptr si no hay
const stardew::maps::MapArea* areaById(const stardew::maps::MapDefinition& map,
                                       const std::string& id);

// El indice en map.areas del area con ese id, o vacio si no hay
std::optional<std::size_t> areaIndex(const stardew::maps::MapDefinition& map,
                                     const std::string& id);

// Si el terreno de la casilla se puede pisar
bool walkable(const stardew::maps::MapDefinition& map, const AreaCell& cell);

// La casilla corrida (dx, dy) dentro de la misma area, o vacio si se sale
std::optional<AreaCell> inside(const stardew::maps::MapDefinition& map, const AreaCell& cell,
                               int dx, int dy);

// El rectangulo que ocupa el area en el mapa general
CellRect areaRect(const stardew::maps::MapArea& area);

// Si los rectangulos pisan alguna area que no este en ignoredAreaIds (las que se estan moviendo)
bool overlapsAreas(const stardew::maps::MapDefinition& map, const std::vector<CellRect>& rects,
                   const std::vector<std::string>& ignoredAreaIds);

// La primera fila del mapa general que queda libre debajo de todas las areas
int bottomOfAreas(const stardew::maps::MapDefinition& map);
