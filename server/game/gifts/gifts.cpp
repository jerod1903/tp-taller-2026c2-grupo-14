#include "server/game/gifts/gifts.h"

#include <cmath>
#include <optional>
#include <string>

#include "server/config/server_config.h"
#include "server/consts.h"
#include "server/game/characters/player.h"
#include "server/game/storage/item_names.h"
#include "server/game/tools/tool.h"

namespace {

ChatMessage notice(const std::string& text) { return ChatMessage{ChatKind::System, "", text}; }

// "semillas de papa (5)"
std::string describe(const InventorySlot& slot) {
    return item_name(slot.item) + " (" + std::to_string(slot.quantity) + ")";
}

bool can_reach(const Player& giver, const Player& receiver) {
    return &giver.get_location() == &receiver.get_location() &&
           std::hypot(giver.get_x() - receiver.get_x(), giver.get_y() - receiver.get_y()) <=
                   server_config().player.gift_reach;
}

}  // namespace

Gifts::Gifts(PlayerCharacters& characters): characters(characters) {}

std::optional<GiftReply> Gifts::offer(Player& giver, const Position& cell) {
    Player* receiver = characters.player_at(giver.get_location(), cell);
    if (receiver == nullptr || receiver == &giver || !can_reach(giver, *receiver)) {
        return std::nullopt;
    }
    uint32_t giver_id = giver.get_id();
    uint8_t slot = giver.get_inventory_state().selectedSlot;
    InventorySlot gift = giver.get_inventory_state().slots.at(slot);
    if (gift.item == ItemType::Empty) {
        return GiftReply{{{giver_id, notice(ServerConstants::GIFT_NOTHING_SELECTED)}}, {}};
    }
    if (is_work_tool(gift.item)) {
        return GiftReply{{{giver_id, notice(ServerConstants::GIFT_NOT_GIFTABLE)}}, {}};
    }
    pending.insert_or_assign(receiver->get_id(), Pending{giver_id, slot, gift});
    return GiftReply{
            {{receiver->get_id(), notice(giver.get_name() + " te quiere regalar " + describe(gift) +
                                         ". " + ServerConstants::GIFT_HOW_TO_ANSWER)},
             {giver_id, notice("Le ofreciste " + describe(gift) + " a " + receiver->get_name())}},
            {}};
}

GiftReply Gifts::answer(Player& receiver, bool accept) {
    uint32_t receiver_id = receiver.get_id();
    auto it = pending.find(receiver_id);
    if (it == pending.end()) {
        return GiftReply{{{receiver_id, notice(ServerConstants::GIFT_NONE_PENDING)}}, {}};
    }
    Pending gift = it->second;
    pending.erase(it);
    Player* giver = characters.find_player(gift.giver_id);
    if (giver == nullptr || giver->get_inventory_state().slots.at(gift.slot) != gift.gift) {
        return GiftReply{{{receiver_id, notice(ServerConstants::GIFT_GONE)}}, {}};
    }
    if (!accept) {
        return GiftReply{
                {{receiver_id, notice("Rechazaste " + describe(gift.gift))},
                 {gift.giver_id, notice(receiver.get_name() + " rechazó " + describe(gift.gift))}},
                {}};
    }
    uint8_t moved = giver->give_slot_to(gift.slot, receiver);
    if (moved == 0) {
        return GiftReply{{{receiver_id, notice(ServerConstants::GIFT_NO_ROOM)}}, {}};
    }
    InventorySlot received{gift.gift.item, moved, gift.gift.quality};
    return GiftReply{
            {{receiver_id, notice(giver->get_name() + " te regaló " + describe(received))},
             {gift.giver_id, notice(receiver.get_name() + " aceptó " + describe(received))}},
            {receiver_id, gift.giver_id}};
}
