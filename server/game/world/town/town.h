#ifndef TOWN_H
#define TOWN_H

#include <memory>
#include <string>

#include "server/game/world/interior.h"
#include "server/game/world/location.h"
#include "server/game/world/position.h"

// TODO: el interior del centro comunitario, enlazado igual que el del mercado.
class Town: public Location {
public:
    // El Town es dueño del interior del mercado, donde están los comerciantes. Enlaza las dos
    // puertas: market_pos en el pueblo <-> la puerta del interior.
    Town(std::string name, Grid grid, const Position& market_pos, std::unique_ptr<Interior> market);

    Interior& get_market();

    // Actualiza también el mercado.
    void update(float dt) override;

private:
    std::unique_ptr<Interior> market;
};

#endif  // TOWN_H
