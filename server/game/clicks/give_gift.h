#ifndef GIVE_GIFT_H
#define GIVE_GIFT_H

#include "server/game/clicks/click_action.h"

// Si en la celda hay otro jugador que le llega, le ofrece la pila que tiene elegida; el otro la
// acepta o la rechaza por el chat.
class GiveGift: public ClickAction {
public:
    bool try_at(Player& player, GameContext& context, const Position& cell) const override;
};

#endif  // GIVE_GIFT_H
