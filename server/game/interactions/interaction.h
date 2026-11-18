#ifndef INTERACTION_H
#define INTERACTION_H

#include <vector>

#include "server/game/interactions/outcome.h"

class Player;
class GameContext;

// Algo que el jugador hace con lo que tiene al alcance al apretar interactuar. Sin estado.
class Interaction {
public:
    // Outcome::not_here() si no hay con qué hacerla al alcance del jugador.
    virtual const Outcome& try_with(Player& player, GameContext& context) const = 0;
    virtual int energy_cost() const = 0;

    virtual ~Interaction() = default;
};

// Todas, en orden de prioridad: se hace la primera que tenga con qué hacerse.
const std::vector<const Interaction*>& interactions();

#endif  // INTERACTION_H
