#include "server/game/tools/sword.h"

#include "server/config/server_config.h"
#include "server/game/commands/game_context.h"

bool Sword::use(Player& player, GameContext& context, const Position& cell) const {
    return context.combat().attack(player, cell);
}

int Sword::energy_cost() const { return server_config().energy.attack; }
