#include "server/game/cheats/seeds_cheat.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <optional>

#include "common/game_dto.h"
#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

void SeedsCheat::apply(Player& player, GameContext& context, const std::string& payload) const {
    std::optional<int> count = amount(payload, server_config().cheats.seeds);
    if (!count) {
        tell(player, context, "Uso: $seeds [cantidad]");
        return;
    }
    uint8_t each = static_cast<uint8_t>(std::min<int>(*count, std::numeric_limits<uint8_t>::max()));
    for (ItemType seeds: {ItemType::StrawberrySeeds, ItemType::ScallionSeeds, ItemType::PotatoSeeds,
                          ItemType::PumpkinSeeds}) {
        player.receive(seeds, each);
    }
    context.sessions().send_to(player.get_id(), player.get_inventory_state());
}
