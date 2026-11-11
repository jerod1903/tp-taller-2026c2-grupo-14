#pragma once

#include <string>
#include <vector>

#include "common/map/MapDefinition.h"

// Consultas sobre la mina: sus pisos y sus entradas. Ninguna cambia el mapa.

// Si el mapa ya tiene algun piso de la mina
bool hasDungeonFloors(const stardew::maps::MapDefinition& map);

// El piso de la mina con el numero mas bajo, o nullptr si no hay pisos
const stardew::maps::MapArea* firstDungeonFloor(const stardew::maps::MapDefinition& map);

// Las entradas a la mina, en el orden del mapa
std::vector<const stardew::maps::MapElement*> dungeonEntrances(
        const stardew::maps::MapDefinition& map);

// Los ids de las areas que se mueven junto con esa: los pisos de la mina se mueven todos juntos;
// cualquier otra, sola
std::vector<std::string> areasMovingWith(const stardew::maps::MapDefinition& map,
                                         const std::string& areaId);
