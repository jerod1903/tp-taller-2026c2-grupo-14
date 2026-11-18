#include "server/game/tools/watering_can.h"

#include "server/config/server_config.h"
#include "server/game/commands/game_context.h"

bool WateringCan::use(Player& player, GameContext& context, const Position& cell) const {
    return context.farming().water(player, cell);
}

int WateringCan::energy_cost() const { return server_config().energy.water; }
