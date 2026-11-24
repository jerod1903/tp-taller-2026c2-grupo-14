#include "server/game/commands/close_chest_command.h"

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

CloseChestCommand::CloseChestCommand(uint32_t player_id): GameCommand(player_id) {}

void CloseChestCommand::execute(GameContext& context) {
    if (Player* player = context.characters().find_player(player_id)) {
        context.storage().close(*player);
    }
}
