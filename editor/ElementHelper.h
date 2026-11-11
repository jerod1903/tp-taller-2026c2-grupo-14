#pragma once

#include <optional>
#include <string>

#include "common/map/MapDefinition.h"
#include "common/map/MapRequirements.h"
#include "editor/MapHelper.h"

// Consultas sobre los elementos del mapa. Ninguna cambia el mapa.

// El elemento que ocupa la casilla, o nullptr si esta libre
const stardew::maps::MapElement* elementAt(const stardew::maps::MapDefinition& map,
                                           const AreaCell& cell);

// El elemento con ese id, o nullptr si no hay
const stardew::maps::MapElement* elementById(const stardew::maps::MapDefinition& map,
                                             const std::string& id);

// El primer elemento de ese tipo en el area, o nullptr si no hay
const stardew::maps::MapElement* elementOfType(const stardew::maps::MapDefinition& map,
                                               const std::string& areaId,
                                               stardew::maps::ElementType type);

// Si algun elemento o transicion ya usa ese id
bool idTaken(const stardew::maps::MapDefinition& map, const std::string& id);

// La primera regla de ese tipo de elemento (de ahi salen su tamano y su capa), o nullptr si no
// hay ninguna
const stardew::maps::ElementRule* ruleFor(const stardew::maps::MapRequirements& requirements,
                                          stardew::maps::ElementType type);

// Donde quedaria la esquina de arriba a la izquierda de una base de width x height apoyada con su
// esquina de abajo a la izquierda en bottomLeft (mapa general). Vacio si la base no cae entera en
// una sola area o pisa otro elemento. El elemento ignoredId no cuenta: es el que se esta moviendo.
std::optional<AreaCell> freeBaseAt(const stardew::maps::MapDefinition& map,
                                   const stardew::maps::MapCell& bottomLeft, int width, int height,
                                   const std::string& ignoredId = "");

// La esquina de abajo a la izquierda de la base del elemento, en el mapa general
stardew::maps::MapCell bottomLeftOf(const stardew::maps::MapDefinition& map,
                                    const stardew::maps::MapElement& element);

// Donde aparece quien llega a un elemento: la primera casilla que se puede pisar y esta libre,
// bajando desde el centro de su base. Vacio si no hay ninguna.
std::optional<AreaCell> inFrontOf(const stardew::maps::MapDefinition& map,
                                  const stardew::maps::MapElement& element);

// Donde aparece quien llega al lado de un elemento (la salida de un piso): la primera casilla
// vecina a su base que se puede pisar y esta libre, en el orden de NEIGHBOURS. Vacio si no hay
// ninguna.
std::optional<AreaCell> besideOf(const stardew::maps::MapDefinition& map,
                                 const stardew::maps::MapElement& element);
