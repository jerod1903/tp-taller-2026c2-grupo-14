#ifndef SPAWN_POINT_H
#define SPAWN_POINT_H

#include "server/game/world/point.h"

class Location;

// Donde aparece un jugador al entrar a la partida: su centro
struct SpawnPoint {
    Location& location;
    Point point;
};

#endif  // SPAWN_POINT_H
