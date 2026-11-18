#ifndef TALK_TO_MERCHANT_H
#define TALK_TO_MERCHANT_H

#include "server/game/clicks/click_action.h"

// Si en la celda hay un comerciante que le llega, pasa a hablar con él y le manda su saludo. La
// energía la cobra el Market.
class TalkToMerchant: public ClickAction {
public:
    bool try_at(Player& player, GameContext& context, const Position& cell) const override;
};

#endif  // TALK_TO_MERCHANT_H
