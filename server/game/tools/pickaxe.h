#ifndef PICKAXE_H
#define PICKAXE_H

#include "server/game/tools/tool.h"

// Da un golpe al mineral de la celda de adelante, solo en la mina.
class Pickaxe: public Tool {
public:
    bool use(Player& player, GameContext& context, const Position& cell) const override;
    int energy_cost() const override;
};

#endif  // PICKAXE_H