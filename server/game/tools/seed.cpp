#include "server/game/tools/seed.h"

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

Seed::Seed(CropType crop): crop(crop) {}

bool Seed::use(Player& player, GameContext& context) const {
    if (!context.farming().plant(player, crop)) {
        return false;
    }
    player.remove_selected(1);
    context.sessions().send_to(player.get_id(), player.get_inventory_state());
    return true;
}

int Seed::energy_cost() const { return server_config().energy.plant; }
