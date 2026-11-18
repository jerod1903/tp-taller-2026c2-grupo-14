#include "server/game/commands/use_tool_command.h"

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"
#include "server/game/tools/tool.h"

UseToolCommand::UseToolCommand(uint32_t player_id): GameCommand(player_id) {}

void UseToolCommand::execute(GameContext& context) {
    Player* player = context.characters().find_player(player_id);
    if (player == nullptr || player->is_unconscious()) {
        return;
    }
    const Tool* tool = tool_for(player->selected_item());
    if (tool != nullptr && tool->use(*player, context)) {
        player->spend_energy(tool->energy_cost());
    }
}
