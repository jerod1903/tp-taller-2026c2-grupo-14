#include "server/game/commands/select_slot_command.h"

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

SelectSlotCommand::SelectSlotCommand(uint32_t player_id, const SelectSlotRequest& request):
        GameCommand(player_id), slot(request.slot) {}

void SelectSlotCommand::execute(GameContext& context) {
    if (Player* player = context.characters().find_player(player_id)) {
        player->select_inventory_slot(slot);
        context.sessions().send_to(player_id, player->get_inventory_state());
    }
}
