#include "server/game/cheats/add_gold_cheat.h"

#include <optional>

#include "server/config/server_config.h"
#include "server/game/characters/player.h"

void AddGoldCheat::apply(Player& player, GameContext& context, const std::string& payload) const {
    std::optional<int> gold = amount(payload, server_config().cheats.add_gold);
    if (!gold) {
        tell(player, context, "Uso: $add_gold [cantidad]");
        return;
    }
    player.add_gold(*gold);
}
