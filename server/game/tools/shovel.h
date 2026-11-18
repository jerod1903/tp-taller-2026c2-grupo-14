#ifndef SHOVEL_H
#define SHOVEL_H

#include "server/game/tools/tool.h"

// Cava la celda de adelante, solo en la granja: pasto → tierra → tierra arada.
class Shovel: public Tool {
public:
    bool use(Player& player, GameContext& context, const Position& cell) const override;
    int energy_cost() const override;
};

#endif  // SHOVEL_H
