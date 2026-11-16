#include "server/game/commands/enter_portal_command.h"

EnterPortalCommand::EnterPortalCommand(uint32_t player_id): GameCommand(player_id) {}

void EnterPortalCommand::execute(GameContext& context) {
    context.movement().enter_portal(player_id);
}
