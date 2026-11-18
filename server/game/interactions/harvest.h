#ifndef HARVEST_H
#define HARVEST_H

#include "server/game/interactions/interaction.h"

// Cosecha el cultivo listo de la celda de adelante, con la calidad que se ganó.
class Harvest: public Interaction {
public:
    const Outcome& try_with(Player& player, GameContext& context) const override;
    int energy_cost() const override;
};

#endif  // HARVEST_H
