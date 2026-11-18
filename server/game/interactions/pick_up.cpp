#include "server/game/interactions/pick_up.h"

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/world/location.h"

const Outcome& PickUp::try_with(Player& player, GameContext&) const {
    return player.get_location().pick_up(player);
}

int PickUp::energy_cost() const { return server_config().energy.pick_up; }
