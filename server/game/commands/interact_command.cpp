#include "server/game/commands/interact_command.h"

#include <cmath>
#include <memory>
#include <vector>

#include "common/game_dto.h"
#include "server/config/server_config.h"
#include "server/game/characters/animals/animal.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"

namespace {

constexpr const char* INVENTORY_FULL = "El inventario está lleno";

// La vaca lista mas cercana al jugador dentro del alcance, o nullptr si no hay ninguna.
// Solo cuenta las de su misma area: las posiciones de areas distintas no se comparan.
Animal* nearest_ready_cow(const std::vector<std::unique_ptr<Animal>>& animals,
                          const Player& player) {
    Animal* nearest = nullptr;
    float nearest_distance = server_config().animals.milking_reach;
    for (const std::unique_ptr<Animal>& animal: animals) {
        if (animal->get_kind() != AnimalKind::Cow || !animal->is_ready() ||
            &animal->get_location() != &player.get_location()) {
            continue;
        }
        float distance =
                std::hypot(animal->get_x() - player.get_x(), animal->get_y() - player.get_y());
        if (distance <= nearest_distance) {
            nearest = animal.get();
            nearest_distance = distance;
        }
    }
    return nearest;
}

}  // namespace

InteractCommand::InteractCommand(uint32_t player_id): GameCommand(player_id) {}

void InteractCommand::execute(GameContext& context) {
    Player* player = context.characters().find_player(player_id);
    if (player == nullptr) {
        return;
    }
    Animal* cow = nearest_ready_cow(context.characters().farm_animals(), *player);
    if (cow == nullptr) {
        return;
    }
    if (player->receive(ItemType::Milk, 1) > 0) {
        context.sessions().send_to(player_id, ChatMessage{ChatKind::System, "", INVENTORY_FULL});
        return;
    }
    cow->milk();
    context.sessions().send_to(player_id, player->get_inventory_state());
}
