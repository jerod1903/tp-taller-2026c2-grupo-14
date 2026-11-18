#include "server/game/commands/click_command.h"

#include <optional>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

ClickCommand::ClickCommand(uint32_t player_id, const ClickRequest& request):
        GameCommand(player_id), cell{request.x, request.y} {}

void ClickCommand::execute(GameContext& context) {
    Player* player = context.characters().find_player(player_id);
    if (player == nullptr) {
        return;
    }
    std::optional<ChatMessage> greeting = context.commerce().talk(*player, cell);
    if (greeting) {
        context.sessions().send_to(player_id, *greeting);
    }
}
