#ifndef TOWN_H
#define TOWN_H

#include <string>

#include "server/game/world/location.h"

// TODO: centro comunitario, entrada a la mazmorra y los MERCHANT_COUNT = 4 mercaderes (el Town
// es dueño de ellos y los registra como entidades de la Location).
class Town: public Location {
public:
    Town(std::string name, Grid grid);

    void update(float dt) override;
};

#endif  // TOWN_H
