#include "server/game/characters/wanderer.h"

#include <array>

namespace {
// Quieto pesa lo mismo que las cuatro direcciones juntas: la mitad del tiempo esta parado.
constexpr std::array<Direction, 8> CHOICES = {Direction::NONE, Direction::NONE, Direction::NONE,
                                              Direction::NONE, Direction::UP,   Direction::DOWN,
                                              Direction::LEFT, Direction::RIGHT};
}  // namespace

Wanderer::Wanderer(std::mt19937& rng, float min_walk, float max_walk):
        rng(rng), min_walk(min_walk), max_walk(max_walk) {}

void Wanderer::wander(Creature& creature, float dt) {
    time_left -= dt;
    if (time_left > 0) {
        return;
    }
    std::uniform_int_distribution<std::size_t> choice(0, CHOICES.size() - 1);
    std::uniform_real_distribution<float> seconds(min_walk, max_walk);
    creature.set_direction(CHOICES[choice(rng)]);
    time_left = seconds(rng);
}
