#ifndef SEEDS_CHEAT_H
#define SEEDS_CHEAT_H

#include <string>

#include "server/game/cheats/cheat.h"

// "$seeds [cantidad]": esa cantidad de semillas de cada cultivo al inventario, las que entren.
// Sin cantidad, la de [cheats] seeds.
class SeedsCheat: public Cheat {
public:
    void apply(Player& player, GameContext& context, const std::string& payload) const override;
};

#endif  // SEEDS_CHEAT_H
