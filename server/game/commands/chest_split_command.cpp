#include "server/game/commands/chest_split_command.h"

#include <optional>

#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

ChestSplitCommand::ChestSplitCommand(uint32_t player_id, const ChestSlotRequest& request):
        GameCommand(player_id), request(request) {}

void ChestSplitCommand::execute(GameContext& context) {
    Player* player = context.characters().find_player(player_id);
    if (player == nullptr || player->is_unconscious()) {
        return;
    }
    std::optional<ChestUpdate> update =
            context.storage().split(*player, request.side, request.slot);
    if (!update) {
        return;
    }
    if (request.side == ChestSide::Player) {
        context.sessions().send_to(player_id, player->get_inventory_state());
        return;
    }
    for (uint32_t viewer: update->viewers) {
        context.sessions().send_to(viewer, update->chest);
    }
}
