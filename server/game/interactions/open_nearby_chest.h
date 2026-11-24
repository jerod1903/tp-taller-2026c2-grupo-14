#ifndef OPEN_NEARBY_CHEST_H
#define OPEN_NEARBY_CHEST_H

#include "server/game/interactions/interaction.h"

// Abre el cofre más cercano, si alguno le llega, y le manda su contenido. No cansa.
class OpenNearbyChest: public Interaction {
public:
    const Outcome& try_with(Player& player, GameContext& context) const override;
    int energy_cost() const override;
};

#endif  // OPEN_NEARBY_CHEST_H
