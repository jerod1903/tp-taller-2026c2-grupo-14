#include "server/game/characters/animals/chicken/chicken.h"

#include "server/config/server_config.h"

Chicken::Chicken(uint32_t id, const Position& pos, Location& location, uint32_t seed):
        Animal(id, pos, location, seed),
        days_left(server_config().animals.chicken.production_days) {}

AnimalKind Chicken::get_kind() const { return AnimalKind::Chicken; }

std::optional<ItemType> Chicken::start_new_day() {
    if (--days_left > 0) {
        return std::nullopt;
    }
    days_left = server_config().animals.chicken.production_days;
    return ItemType::Egg;
}
