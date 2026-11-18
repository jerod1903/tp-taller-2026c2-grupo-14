#include "server/game/commands/click_command.h"

#include <algorithm>
#include <vector>

#include "server/game/characters/player.h"
#include "server/game/clicks/click_action.h"
#include "server/game/commands/game_context.h"

ClickCommand::ClickCommand(uint32_t player_id, const ClickRequest& click):
        GameCommand(player_id), cell{static_cast<int>(click.x), static_cast<int>(click.y)} {}

void ClickCommand::execute(GameContext& context) {
    Player* player = context.characters().find_player(player_id);
    if (player == nullptr || player->is_unconscious()) {
        return;
    }
    const std::vector<const ClickAction*>& all = click_actions();
    std::any_of(all.begin(), all.end(), [&](const ClickAction* action) {
        return action->try_at(*player, context, cell);
    });
}
