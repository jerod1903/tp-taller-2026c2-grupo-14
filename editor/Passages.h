#pragma once

#include "common/map/MapDefinition.h"

// Un pasaje une dos areas vecinas en el mapa general: un porton a cada lado del borde y una
// transicion en cada sentido, que deja al jugador una casilla adentro del otro lado.
//
// (x, y) es una casilla del mapa general (con los offsets de las areas). Si ahi hay un porton de
// un pasaje, lo quita con su par y sus transiciones. Si no, y la casilla esta en el borde con
// otra area, crea el pasaje. Devuelve false si no cambio nada: la casilla no da a otra area, esta
// ocupada o alguno de los lados no se puede pisar.
bool togglePassage(stardew::maps::MapDefinition& map, int x, int y);
