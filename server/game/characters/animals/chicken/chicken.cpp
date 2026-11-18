#include "server/game/characters/animals/chicken/chicken.h"

#include <utility>

#include "server/config/server_config.h"

Chicken::Chicken(uint32_t id, const Position& pos, Location& location, uint32_t seed):
        Animal(id, pos, location, seed),
        days_left(server_config().animals.chicken.production_days) {
    schedule_egg();
}

AnimalKind Chicken::get_kind() const { return AnimalKind::Chicken; }

void Chicken::start_new_day() {
    if (lay_in) {
        lay_in.reset();
        laid = true;
    }
    if (--days_left <= 0) {
        days_left = server_config().animals.chicken.production_days;
        schedule_egg();
    }
}

std::optional<ItemType> Chicken::take_drop() {
    if (!std::exchange(laid, false)) {
        return std::nullopt;
    }
    return ItemType::Egg;
}

void Chicken::update(float dt) {
    Animal::update(dt);
    if (lay_in && (*lay_in -= dt) <= 0) {
        lay_in.reset();
        laid = true;
    }
}

void Chicken::schedule_egg() {
    std::uniform_real_distribution<float> moment(0.0f, server_config().game.day_speed);
    lay_in = moment(rng);
}
