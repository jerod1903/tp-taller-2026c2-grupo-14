#ifndef SWORD_H
#define SWORD_H

#include "server/game/tools/tool.h"

// Le pega al enemigo de la celda clickeada, pegada a la del jugador, en la mazmorra.
class Sword: public Tool {
public:
    bool use(Player& player, GameContext& context, const Position& cell) const override;
    int energy_cost() const override;
};

#endif  // SWORD_H
