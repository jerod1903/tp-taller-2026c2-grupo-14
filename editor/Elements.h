#pragma once

#include <string>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"

// Quita el elemento con ese id y las transiciones que dispara. Devuelve false si no habia
// ninguno con ese id.
bool removeElement(stardew::maps::MapDefinition& map, const std::string& id);

// Pone un elemento con el tamano y la capa de la regla, apoyado con la esquina de abajo a la
// izquierda de su base en bottomLeft (mapa general). Su id es el tipo y el primer numero libre
// (market-1). Devuelve false si la base no cae entera en una sola area o pisa otro elemento.
bool placeElement(stardew::maps::MapDefinition& map, const stardew::maps::ElementRule& rule,
                  const stardew::maps::MapCell& bottomLeft);
