#include "server/game/world/farm/milking.h"

#include <cmath>

#include "server/config/server_config.h"
#include "server/game/characters/animals/cow/cow.h"

namespace {

// Solo cuenta las de su misma área: las posiciones de áreas distintas no se comparan.
Cow* nearest_ready_cow(const std::vector<std::unique_ptr<Animal>>& animals, const Player& player) {
    Cow* nearest = nullptr;
    float nearest_distance = server_config().animals.milking_reach;
    for (const std::unique_ptr<Animal>& animal: animals) {
        Cow* cow = dynamic_cast<Cow*>(animal.get());
        if (cow == nullptr || !cow->is_ready() || &cow->get_location() != &player.get_location()) {
            continue;
        }
        float distance = std::hypot(cow->get_x() - player.get_x(), cow->get_y() - player.get_y());
        if (distance <= nearest_distance) {
            nearest = cow;
            nearest_distance = distance;
        }
    }
    return nearest;
}

}  // namespace

const Outcome& milk_nearest_cow(const std::vector<std::unique_ptr<Animal>>& animals,
                                Player& player) {
    Cow* cow = nearest_ready_cow(animals, player);
    if (cow == nullptr) {
        return Outcome::not_here();
    }
    if (player.receive(ItemType::Milk, 1) > 0) {
        return Outcome::inventory_full();
    }
    cow->milk();
    return Outcome::done();
}
