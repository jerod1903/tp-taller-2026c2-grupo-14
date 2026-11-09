#pragma once

#include <cstddef>
#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

// Consultas al mapa que comparten el editor y sus herramientas. Ninguna cambia el mapa.

// Las casillas que se dejan entre las areas cuando el editor las acomoda
constexpr int AREA_GAP = 2;

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

// La primera regla de ese tipo de elemento (de ahi salen su tamano y su capa), o nullptr si no
// hay ninguna
const stardew::maps::ElementRule* ruleFor(const stardew::maps::MapRequirements& requirements,
                                          stardew::maps::ElementType type);

// Donde quedaria la esquina de arriba a la izquierda de una base de width x height apoyada con su
// esquina de abajo a la izquierda en bottomLeft (mapa general). Vacio si la base no cae entera en
// una sola area o pisa otro elemento.
std::optional<AreaCell> freeBaseAt(const stardew::maps::MapDefinition& map,
                                   const stardew::maps::MapCell& bottomLeft, int width, int height);

// El primer elemento de ese tipo en el area, o nullptr si no hay
const stardew::maps::MapElement* elementOfType(const stardew::maps::MapDefinition& map,
                                               const std::string& areaId,
                                               stardew::maps::ElementType type);

// La primera fila del mapa general que queda libre debajo de todas las areas
int bottomOfAreas(const stardew::maps::MapDefinition& map);

// Si el mapa ya tiene algun piso de la mina
bool hasDungeonFloors(const stardew::maps::MapDefinition& map);

// El piso de la mina con el numero mas bajo, o nullptr si no hay pisos
const stardew::maps::MapArea* firstDungeonFloor(const stardew::maps::MapDefinition& map);

// Donde aparece quien llega a un elemento: la primera casilla que se puede pisar y esta libre,
// bajando desde el centro de su base. Vacio si no hay ninguna.
std::optional<AreaCell> inFrontOf(const stardew::maps::MapDefinition& map,
                                  const stardew::maps::MapElement& element);
