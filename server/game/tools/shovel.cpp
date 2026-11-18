#include "server/game/tools/shovel.h"

#include "server/config/server_config.h"
#include "server/game/commands/game_context.h"

bool Shovel::use(Player& player, GameContext& context, const Position& cell) const {
    return context.farming().dig(player, cell);
}

int Shovel::energy_cost() const { return server_config().energy.dig; }
