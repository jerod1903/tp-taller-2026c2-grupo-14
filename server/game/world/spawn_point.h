#ifndef SPAWN_POINT_H
#define SPAWN_POINT_H

#include "server/game/world/position.h"

class Location;

// Donde aparece un jugador al entrar a la partida
struct SpawnPoint {
    Location& location;
    Position position;
};

#endif  // SPAWN_POINT_H
