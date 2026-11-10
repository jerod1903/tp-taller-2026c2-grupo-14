#pragma once

#include <string>
#include <vector>

#include "common/map/MapDefinition.h"
#include "editor/MapHelper.h"

// Corre esas areas (dx, dy) en el mapa general. Sus casillas, elementos y transiciones van en
// coordenadas del area, asi que viajan con ella: solo cambia su posicion. Antes quita los pasajes
// que quedarian separados.
void moveAreas(stardew::maps::MapDefinition& map, const std::vector<std::string>& areaIds, int dx,
               int dy);

// Agrega un area vacia (todo pasto) de ese tipo y tamano, con su esquina de arriba a la izquierda
// en topLeft (mapa general). Su id es "granja" o "pueblo", o el primer numero libre (granja-2).
// Devuelve el id.
std::string addArea(stardew::maps::MapDefinition& map, stardew::maps::AreaType type,
                    const AreaSize& size, const stardew::maps::MapCell& topLeft);

// Quita esas areas con sus elementos, las transiciones que salen de ellas y las que llegan. De sus
// pasajes se va tambien el porton del otro lado.
void removeAreas(stardew::maps::MapDefinition& map, const std::vector<std::string>& areaIds);
