#include "server/game/tools/shovel.h"

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"
#include "server/game/world/farm/farm.h"

bool Shovel::use(Player& player, GameContext& context) const {
    Farm* farm = context.characters().find_farm();
    return farm != nullptr && &player.get_location() == farm && farm->dig(player.facing_cell());
}

int Shovel::energy_cost() const { return server_config().energy.dig; }
