#ifndef USE_TOOL_H
#define USE_TOOL_H

#include "server/game/clicks/click_action.h"

// Usa en la celda lo que tiene en el slot elegido, si es una herramienta (tool_for).
class UseTool: public ClickAction {
public:
    bool try_at(Player& player, GameContext& context, const Position& cell) const override;
};

#endif  // USE_TOOL_H
