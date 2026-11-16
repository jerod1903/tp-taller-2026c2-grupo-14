#include "server/game/commands/typing_command.h"

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

TypingCommand::TypingCommand(uint32_t player_id, const TypingRequest& request):
        GameCommand(player_id), typing(request.typing) {}

void TypingCommand::execute(GameContext& context) {
    if (Player* player = context.find_player(player_id)) {
        player->set_typing(typing);
    }
}
