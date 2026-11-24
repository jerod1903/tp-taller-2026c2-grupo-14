#ifndef GIFT_REPLY_H
#define GIFT_REPLY_H

#include "server/game/commands/game_context.h"
#include "server/game/commands/player_gifts.h"

// Manda los avisos de un regalo y el inventario nuevo a quienes les cambió.
void send_gift_reply(GameContext& context, const GiftReply& reply);

#endif  // GIFT_REPLY_H
