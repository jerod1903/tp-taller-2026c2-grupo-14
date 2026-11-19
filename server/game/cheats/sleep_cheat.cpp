#include "server/game/cheats/sleep_cheat.h"

#include "server/game/commands/game_context.h"

void SleepCheat::apply(Player& player, GameContext& context, const std::string& payload) const {
    if (!payload.empty()) {
        tell(player, context, "Uso: $sleep");
        return;
    }
    context.day().end_day();
}
