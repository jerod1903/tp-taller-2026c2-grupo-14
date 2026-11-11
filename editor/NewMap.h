#pragma once

#include <string>

#include "common/map/MapDefinition.h"

// El mapa con el que arranca "Nuevo mapa": una granja y un pueblo de 30 x 20, todo pasto, uno al
// lado del otro y unidos por un pasaje en el medio del borde. Lo demas (la entrada a la mina, los
// edificios, otras areas) se pone desde el editor.
stardew::maps::MapDefinition newMap(const std::string& name, int formatVersion);
