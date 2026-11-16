#include "server/game/commands/leave_command.h"

LeaveCommand::LeaveCommand(uint32_t player_id): GameCommand(player_id) {}

void LeaveCommand::execute(GameContext& context) { context.sessions().remove_player(player_id); }
