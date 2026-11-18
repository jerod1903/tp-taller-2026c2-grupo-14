#include "server/game/interactions/outcome.h"

#include "common/game_dto.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

namespace {

constexpr const char* INVENTORY_FULL = "El inventario está lleno";

class NotHere: public Outcome {
public:
    bool apply(Player&, GameContext&, int) const override { return false; }
};

class InventoryFull: public Outcome {
public:
    bool apply(Player& player, GameContext& context, int) const override {
        context.sessions().send_to(player.get_id(),
                                   ChatMessage{ChatKind::System, "", INVENTORY_FULL});
        return true;
    }
};

class Done: public Outcome {
public:
    bool apply(Player& player, GameContext& context, int energy_cost) const override {
        player.spend_energy(energy_cost);
        context.sessions().send_to(player.get_id(), player.get_inventory_state());
        return true;
    }
};

}  // namespace

const Outcome& Outcome::not_here() {
    static const NotHere outcome;
    return outcome;
}

const Outcome& Outcome::inventory_full() {
    static const InventoryFull outcome;
    return outcome;
}

const Outcome& Outcome::done() {
    static const Done outcome;
    return outcome;
}
