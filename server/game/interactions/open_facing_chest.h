#ifndef OPEN_FACING_CHEST_H
#define OPEN_FACING_CHEST_H

#include "server/game/interactions/interaction.h"

// Abre el cofre de la celda de adelante, si le llega, y le manda su contenido. No cansa.
class OpenFacingChest: public Interaction {
public:
    const Outcome& try_with(Player& player, GameContext& context) const override;
    int energy_cost() const override;
};

#endif  // OPEN_FACING_CHEST_H
