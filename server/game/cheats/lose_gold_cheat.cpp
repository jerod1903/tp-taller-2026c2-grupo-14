#include "server/game/cheats/lose_gold_cheat.h"

#include <optional>

#include "server/config/server_config.h"
#include "server/game/characters/player.h"

void LoseGoldCheat::apply(Player& player, GameContext& context, const std::string& payload) const {
    std::optional<int> gold = amount(payload, server_config().cheats.lose_gold);
    if (!gold) {
        tell(player, context, "Uso: $lose_gold [cantidad]");
        return;
    }
    player.spend_gold(*gold);
}
