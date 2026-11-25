#ifndef ADD_GOLD_CHEAT_H
#define ADD_GOLD_CHEAT_H

#include <string>

#include "server/game/cheats/cheat.h"

// "$add_gold [cantidad]": suma esa cantidad de oro. Sin cantidad, suma la de [cheats] add_gold.
class AddGoldCheat: public Cheat {
public:
    void apply(Player& player, GameContext& context, const std::string& payload) const override;
};

#endif  // ADD_GOLD_CHEAT_H
