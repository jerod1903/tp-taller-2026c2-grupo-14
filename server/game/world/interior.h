#ifndef INTERIOR_H
#define INTERIOR_H

#include <string>

#include "server/game/world/location.h"
#include "server/game/world/position.h"

// Un edificio por dentro (la cabaña, el mercado): una Location propia, con una puerta que lleva
// afuera. Vive dentro de la Location donde está el edificio.
class Interior: public Location {
public:
    Interior(std::string name, Grid grid, const Position& door_pos);

    // Tile interior que lleva afuera.
    const Position& get_door_pos() const;

    // Enlaza su puerta con la del edificio en `outside`; las dos se usan con la tecla. Se sale un
    // tile y medio debajo de la puerta del edificio, para que su dibujo no tape al jugador.
    void link_to(Location& outside, const Position& building_door);

    void update(float dt) override;

private:
    Position door_pos;
};

#endif  // INTERIOR_H
