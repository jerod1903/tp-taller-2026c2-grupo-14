#include "server/game/clicks/give_gift.h"

#include <optional>

#include "server/game/commands/game_context.h"
#include "server/game/commands/gift_reply.h"

bool GiveGift::try_at(Player& player, GameContext& context, const Position& cell) const {
    std::optional<GiftReply> reply = context.gifts().offer(player, cell);
    if (!reply) {
        return false;
    }
    send_gift_reply(context, *reply);
    return true;
}
