#pragma once

#include <string>

#include "common/map/MapDefinition.h"

// Quita el elemento con ese id y las transiciones que dispara. Devuelve false si no habia
// ninguno con ese id.
bool removeElement(stardew::maps::MapDefinition& map, const std::string& id);
