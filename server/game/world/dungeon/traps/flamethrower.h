#ifndef FLAMETHROWER_H
#define FLAMETHROWER_H

#include <cstdint>
#include <vector>

#include "server/game/world/direction.h"
#include "server/game/world/dungeon/traps/trap.h"

// Lanzallamas pegado a una pared: está en la celda de al lado de la pared y tira un chorro de
// fuego para el otro lado (`facing`: abajo, izquierda o derecha). Prendido, lastima a quien esté
// en su celda o en las que siguen, hasta `reach` celdas o hasta una pared. Sus valores salen de
// [traps.flamethrower] en server_config.toml.
class Flamethrower: public Trap {
public:
    Flamethrower(uint32_t id, const Position& pos, Location& location,
                 const std::vector<Creature*>& targets, Direction facing, float start);

    EntityType entity_type() const override;

protected:
    bool covers(const Position& cell) const override;

private:
    Direction facing;
    int reach;
};

#endif  // FLAMETHROWER_H
