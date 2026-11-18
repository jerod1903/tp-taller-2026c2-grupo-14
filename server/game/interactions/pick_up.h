#ifndef PICK_UP_H
#define PICK_UP_H

#include "server/game/interactions/interaction.h"

// Levanta lo que haya tirado en el piso (un huevo, madera) en la celda del jugador o en la de
// adelante, en cualquier área.
class PickUp: public Interaction {
public:
    const Outcome& try_with(Player& player, GameContext& context) const override;
    int energy_cost() const override;
};

#endif  // PICK_UP_H
