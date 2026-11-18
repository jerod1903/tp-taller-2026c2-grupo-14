#include "server/game/characters/animals/cow/cow.h"

#include "server/config/server_config.h"

Cow::Cow(uint32_t id, const Position& pos, Location& location, uint32_t seed):
        Animal(id, pos, location, seed),
        days_left(server_config().animals.cow.production_days) {}

AnimalKind Cow::get_kind() const { return AnimalKind::Cow; }

bool Cow::is_ready() const { return ready; }

void Cow::start_new_day() {
    if (!ready && --days_left <= 0) {
        ready = true;
    }
}

void Cow::milk() {
    ready = false;
    days_left = server_config().animals.cow.production_days;
}
