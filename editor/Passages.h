#pragma once

#include <string>
#include <vector>

#include "common/map/MapDefinition.h"

// Un pasaje une dos areas vecinas en el mapa general: un porton a cada lado del borde y una
// transicion en cada sentido, que deja al jugador una casilla adentro del otro lado.
//
// (x, y) es una casilla del mapa general (con los offsets de las areas). Si ahi hay un porton de
// un pasaje, lo quita con su par y sus transiciones. Si no, y la casilla esta en el borde con
// otra area, crea el pasaje. Devuelve false si no cambio nada: la casilla no da a otra area, esta
// ocupada o alguno de los lados no se puede pisar.
bool togglePassage(stardew::maps::MapDefinition& map, int x, int y);

// Quita el porton con ese id, su par del otro lado del borde (si esta) y las transiciones de los
// dos. Devuelve false si ese id no es un porton.
bool removePassage(stardew::maps::MapDefinition& map, const std::string& doorId);

// Los pasajes que quedarian separados si las areas con esos ids se corren (dx, dy) en el mapa
// general: sus dos portones dejan de estar uno al lado del otro. Devuelve un porton de cada uno.
std::vector<std::string> passagesBrokenBy(const stardew::maps::MapDefinition& map,
                                          const std::vector<std::string>& areaIds, int dx, int dy);
