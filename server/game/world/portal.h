#ifndef PORTAL_H
#define PORTAL_H

#include "server/game/world/position.h"

class Location;

struct Portal {
    Position origin;
    Location* target;
    Position arrival;
};

#endif  // PORTAL_H
