#ifndef TOWN_H
#define TOWN_H

#include <memory>
#include <string>

#include "server/game/world/interior.h"
#include "server/game/world/location.h"
#include "server/game/world/position.h"

class Town: public Location {
public:
    // El Town es dueño de los interiores del mercado, donde están los comerciantes, y del centro
    // comunitario, donde se entregan los sets. Enlaza las puertas de cada uno: la del edificio en
    // el pueblo (market_pos, community_center_pos) <-> la del interior.
    Town(std::string name, Grid grid, const Position& market_pos, std::unique_ptr<Interior> market,
         const Position& community_center_pos, std::unique_ptr<Interior> community_center);

    Interior& get_market();
    Interior& get_community_center();

    // Actualiza también los interiores.
    void update(float dt) override;

private:
    std::unique_ptr<Interior> market;
    std::unique_ptr<Interior> community_center;
};

#endif  // TOWN_H
