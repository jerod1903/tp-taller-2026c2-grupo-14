#include "server/game/interactions/harvest.h"

#include "server/config/server_config.h"
#include "server/game/commands/game_context.h"

const Outcome& Harvest::try_with(Player& player, GameContext& context) const {
    return context.farming().harvest(player);
}

int Harvest::energy_cost() const { return server_config().energy.harvest; }
