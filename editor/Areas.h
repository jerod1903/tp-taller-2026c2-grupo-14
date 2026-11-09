#pragma once

#include <string>
#include <vector>

#include "common/map/MapDefinition.h"

// Corre esas areas (dx, dy) en el mapa general. Sus casillas, elementos y transiciones van en
// coordenadas del area, asi que viajan con ella: solo cambia su posicion. Antes quita los pasajes
// que quedarian separados.
void moveAreas(stardew::maps::MapDefinition& map, const std::vector<std::string>& areaIds, int dx,
               int dy);
