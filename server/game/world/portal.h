#ifndef PORTAL_H
#define PORTAL_H

#include "server/game/world/point.h"
#include "server/game/world/position.h"

class Location;

// Los pasajes entre zonas se cruzan caminando; las puertas se usan con la tecla de acción.
enum class PortalTrigger { Step, Action };

struct Portal {
    Position origin;
    Location* target;
    Point arrival;  // donde queda el centro del que llega
    PortalTrigger trigger = PortalTrigger::Step;
};

#endif  // PORTAL_H
