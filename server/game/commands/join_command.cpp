#include "server/game/commands/join_command.h"

#include <utility>

JoinCommand::JoinCommand(uint32_t player_id, GameMember member, std::shared_ptr<Outbox> outbox):
        GameCommand(player_id), member(std::move(member)), outbox(std::move(outbox)) {}

void JoinCommand::execute(GameContext& context) { context.add_player(player_id, member, outbox); }
