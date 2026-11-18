#include "server/game/tools/shovel.h"

#include "server/config/server_config.h"
#include "server/game/commands/game_context.h"

bool Shovel::use(Player& player, GameContext& context) const {
    return context.farming().dig(player);
}

int Shovel::energy_cost() const { return server_config().energy.dig; }
