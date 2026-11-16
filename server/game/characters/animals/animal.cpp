#include "server/game/characters/animals/animal.h"

#include <array>

#include "server/config/server_config.h"

namespace {
// Quieto pesa lo mismo que las cuatro direcciones juntas: la mitad del tiempo esta parado.
constexpr std::array<Direction, 8> CHOICES = {Direction::NONE, Direction::NONE, Direction::NONE,
                                              Direction::NONE, Direction::UP,   Direction::DOWN,
                                              Direction::LEFT, Direction::RIGHT};
}  // namespace

Animal::Animal(uint32_t id, AnimalKind kind, const Position& pos, Location& location,
               uint32_t seed):
        Creature(id, pos, location, server_config().animals.health, server_config().animals.speed),
        kind(kind),
        rng(seed) {}

AnimalKind Animal::get_kind() const { return kind; }

void Animal::update(float dt) {
    time_left -= dt;
    if (time_left <= 0) {
        choose_direction();
    }
    advance(dt);
}

void Animal::choose_direction() {
    std::uniform_int_distribution<std::size_t> choice(0, CHOICES.size() - 1);
    std::uniform_real_distribution<float> seconds(server_config().animals.min_walk,
                                                  server_config().animals.max_walk);
    set_direction(CHOICES[choice(rng)]);
    time_left = seconds(rng);
}
