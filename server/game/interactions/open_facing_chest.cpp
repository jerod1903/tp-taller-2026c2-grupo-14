#include "server/game/interactions/open_facing_chest.h"

#include <optional>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

const Outcome& OpenFacingChest::try_with(Player& player, GameContext& context) const {
    std::optional<ChestState> chest = context.storage().open(player, player.facing_cell());
    if (!chest) {
        return Outcome::not_here();
    }
    context.sessions().send_to(player.get_id(), *chest);
    return Outcome::done();
}

int OpenFacingChest::energy_cost() const { return 0; }
