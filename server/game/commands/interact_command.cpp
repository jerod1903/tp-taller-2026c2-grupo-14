#include "server/game/commands/interact_command.h"

#include <algorithm>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"
#include "server/game/interactions/interaction.h"

InteractCommand::InteractCommand(uint32_t player_id): GameCommand(player_id) {}

void InteractCommand::execute(GameContext& context) {
    Player* player = context.characters().find_player(player_id);
    if (player == nullptr || player->is_exhausted()) {
        return;
    }
    const std::vector<const Interaction*>& all = interactions();
    std::any_of(all.begin(), all.end(), [&](const Interaction* interaction) {
        return interaction->try_with(*player, context)
                .apply(*player, context, interaction->energy_cost());
    });
}
