#include "server/game/interactions/milk.h"

#include "server/config/server_config.h"
#include "server/game/commands/game_context.h"

const Outcome& Milk::try_with(Player& player, GameContext& context) const {
    return context.farming().milk(player);
}

int Milk::energy_cost() const { return server_config().energy.milk; }
