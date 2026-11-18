#ifndef GOLD_CHEAT_H
#define GOLD_CHEAT_H

#include <string>

#include "server/game/cheats/cheat.h"

// "$gold [cantidad]": suma esa cantidad de oro. Sin cantidad, la de [cheats] gold.
class GoldCheat: public Cheat {
public:
    void apply(Player& player, GameContext& context, const std::string& payload) const override;
};

#endif  // GOLD_CHEAT_H
