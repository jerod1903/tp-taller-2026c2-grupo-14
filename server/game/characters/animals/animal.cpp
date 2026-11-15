#include "server/game/characters/animals/animal.h"

#include <array>

#include "server/consts.h"

namespace {
// Quieto pesa lo mismo que las cuatro direcciones juntas: la mitad del tiempo esta parado.
constexpr std::array<Direction, 8> CHOICES = {Direction::NONE, Direction::NONE, Direction::NONE,
                                              Direction::NONE, Direction::UP,   Direction::DOWN,
                                              Direction::LEFT, Direction::RIGHT};
}  // namespace

Animal::Animal(uint32_t id, AnimalKind kind, const Position& pos, Location& location,
               uint32_t seed):
        Creature(id, pos, location, ServerConstants::ANIMAL_HEALTH, ServerConstants::ANIMAL_SPEED),
        kind(kind),
        time_to_ready(ServerConstants::COW_READY_SECONDS),
        rng(seed) {}

AnimalKind Animal::get_kind() const { return kind; }

bool Animal::is_ready() const { return ready; }

void Animal::milk() {
    ready = false;
    time_to_ready = ServerConstants::COW_READY_SECONDS;
}

void Animal::update(float dt) {
    if (kind == AnimalKind::Cow && !ready) {
        time_to_ready -= dt;
        ready = time_to_ready <= 0;
    }
    time_left -= dt;
    if (time_left <= 0) {
        choose_direction();
    }
    advance(dt);
}

void Animal::choose_direction() {
    std::uniform_int_distribution<std::size_t> choice(0, CHOICES.size() - 1);
    std::uniform_real_distribution<float> seconds(ServerConstants::ANIMAL_MIN_WALK,
                                                  ServerConstants::ANIMAL_MAX_WALK);
    set_direction(CHOICES[choice(rng)]);
    time_left = seconds(rng);
}
