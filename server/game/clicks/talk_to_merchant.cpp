#include "server/game/clicks/talk_to_merchant.h"

#include <optional>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

bool TalkToMerchant::try_at(Player& player, GameContext& context, const Position& cell) const {
    std::optional<ChatMessage> greeting = context.commerce().talk(player, cell);
    if (!greeting) {
        return false;
    }
    context.sessions().send_to(player.get_id(), *greeting);
    return true;
}
