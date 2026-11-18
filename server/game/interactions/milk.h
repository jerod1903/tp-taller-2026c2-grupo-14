#ifndef MILK_H
#define MILK_H

#include "server/game/interactions/interaction.h"

// Ordeña la vaca lista más cercana al alcance del jugador.
class Milk: public Interaction {
public:
    const Outcome& try_with(Player& player, GameContext& context) const override;
    int energy_cost() const override;
};

#endif  // MILK_H
