#include "server/game/clicks/use_tool.h"

#include "server/game/characters/player.h"
#include "server/game/tools/tool.h"

bool UseTool::try_at(Player& player, GameContext& context, const Position& cell) const {
    const Tool* tool = tool_for(player.selected_item());
    if (tool == nullptr || !tool->use(player, context, cell)) {
        return false;
    }
    player.spend_energy(tool->energy_cost());
    return true;
}
