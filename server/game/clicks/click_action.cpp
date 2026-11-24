#include "server/game/clicks/click_action.h"

#include "server/game/clicks/give_gift.h"
#include "server/game/clicks/talk_to_merchant.h"
#include "server/game/clicks/use_tool.h"

// Va última: lo que haya en la celda (un comerciante, otro jugador) gana sobre la herramienta.
const std::vector<const ClickAction*>& click_actions() {
    static const TalkToMerchant talk_to_merchant;
    static const GiveGift give_gift;
    static const UseTool use_tool;
    static const std::vector<const ClickAction*> all = {&talk_to_merchant, &give_gift, &use_tool};
    return all;
}
