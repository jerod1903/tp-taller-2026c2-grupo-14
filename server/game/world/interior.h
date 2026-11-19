#ifndef INTERIOR_H
#define INTERIOR_H

#include <string>
#include <vector>

#include "server/game/world/location.h"
#include "server/game/world/position.h"

// Las celdas de la puerta de un edificio, de izquierda a derecha: la del mercado y la del centro
// comunitario son dobles.
using DoorCells = std::vector<Position>;

// Un edificio por dentro (la cabaña, el mercado): una Location propia, con una puerta que lleva
// afuera. Vive dentro de la Location donde está el edificio.
class Interior: public Location {
public:
    Interior(std::string name, Grid grid, const Position& door_pos);

    // Tile interior que lleva afuera.
    const Position& get_door_pos() const;

    // Enlaza su puerta con la del edificio en `outside`: se entra por cualquier celda de esa puerta
    // y las dos se usan con la tecla. Se sale un tile y medio debajo del centro de la puerta del
    // edificio, para que su dibujo no tape al jugador.
    void link_to(Location& outside, const DoorCells& building_door);

    void update(float dt) override;

private:
    Position door_pos;
};

#endif  // INTERIOR_H
