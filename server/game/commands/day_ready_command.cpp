#include "server/game/commands/day_ready_command.h"

#include "server/game/commands/game_context.h"

DayReadyCommand::DayReadyCommand(uint32_t player_id): GameCommand(player_id) {}

void DayReadyCommand::execute(GameContext& context) { context.day().ready_for_next_day(player_id); }

bool DayReadyCommand::runs_while_day_ended() const { return true; }
