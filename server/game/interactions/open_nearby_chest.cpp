#include "server/game/interactions/open_nearby_chest.h"

#include <optional>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

const Outcome& OpenNearbyChest::try_with(Player& player, GameContext& context) const {
    std::optional<ChestState> chest = context.storage().open(player);
    if (!chest) {
        return Outcome::not_here();
    }
    context.sessions().send_to(player.get_id(), *chest);
    return Outcome::done();
}

int OpenNearbyChest::energy_cost() const { return 0; }
