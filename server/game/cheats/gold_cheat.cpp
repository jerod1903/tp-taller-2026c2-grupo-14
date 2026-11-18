#include "server/game/cheats/gold_cheat.h"

#include <optional>

#include "server/config/server_config.h"
#include "server/game/characters/player.h"

void GoldCheat::apply(Player& player, GameContext& context, const std::string& payload) const {
    std::optional<int> gold = amount(payload, server_config().cheats.gold);
    if (!gold) {
        tell(player, context, "Uso: $gold [cantidad]");
        return;
    }
    // El oro viaja en cada snapshot: no hace falta mandar nada más
    player.add_gold(*gold);
}
