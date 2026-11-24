#include "server/game/commands/gift_reply.h"

#include <cstdint>
#include <utility>

#include "server/game/characters/player.h"

void send_gift_reply(GameContext& context, const GiftReply& reply) {
    for (const auto& [player_id, message]: reply.messages) {
        context.sessions().send_to(player_id, message);
    }
    for (uint32_t player_id: reply.inventory_changed) {
        if (Player* player = context.characters().find_player(player_id)) {
            context.sessions().send_to(player_id, player->get_inventory_state());
        }
    }
}
