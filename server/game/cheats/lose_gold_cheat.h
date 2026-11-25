#ifndef LOSE_GOLD_CHEAT_H
#define LOSE_GOLD_CHEAT_H

#include <string>

#include "server/game/cheats/cheat.h"

// "$lose_gold [cantidad]": resta esa cantidad de oro. Sin cantidad, resta la de [cheats] lose_gold.
class LoseGoldCheat: public Cheat {
public:
    void apply(Player& player, GameContext& context, const std::string& payload) const override;
};

#endif  // LOSE_GOLD_CHEAT_H
