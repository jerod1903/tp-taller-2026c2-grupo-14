#pragma once

#include <string>

#include "common/map/MapRequirements.h"

// Lo que tiene que tener un mapa para ser valido, en castellano y un renglon por idea: cuantas
// areas de cada tipo y que elementos necesita cada una. Sale de las reglas, asi no se desactualiza.
// El editor deja hacer otra cosa, pero el mapa no pasa la validacion.
std::string mapGuidelines(const stardew::maps::MapRequirements& requirements);
