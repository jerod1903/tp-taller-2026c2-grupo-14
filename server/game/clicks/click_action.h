#ifndef CLICK_ACTION_H
#define CLICK_ACTION_H

#include <vector>

#include "server/game/world/position.h"

class Player;
class GameContext;

// Lo que pasa cuando el jugador clickea una celda, según lo que haya en ella. Sin estado.
class ClickAction {
public:
    // false si en esa celda no hay con qué hacerla: se prueba la siguiente. Cobra su energía.
    virtual bool try_at(Player& player, GameContext& context, const Position& cell) const = 0;

    virtual ~ClickAction() = default;
};

// Todas, en orden de prioridad: se hace la primera que aplique en la celda.
const std::vector<const ClickAction*>& click_actions();

#endif  // CLICK_ACTION_H
