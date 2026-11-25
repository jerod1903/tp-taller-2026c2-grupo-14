#include "server/game/cheats/finish_cheat.h"

#include "common/game_dto.h"
#include "server/game/commands/game_context.h"

void FinishCheat::apply(Player& player, GameContext& context, const std::string& payload) const {
    if (!payload.empty()) {
        tell(player, context, "Uso: $finish");
        return;
    }
    context.sessions().broadcast(GameCompleted{});
}
