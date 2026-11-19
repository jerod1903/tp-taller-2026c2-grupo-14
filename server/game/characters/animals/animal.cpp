#include "server/game/characters/animals/animal.h"

#include "server/config/server_config.h"

Animal::Animal(uint32_t id, const Position& pos, Location& location, uint32_t seed):
        Creature(id, pos, location, server_config().animals.health, server_config().animals.speed),
        rng(seed),
        wanderer(rng, server_config().animals.min_walk, server_config().animals.max_walk) {}

bool Animal::is_ready() const { return false; }

std::optional<ItemType> Animal::take_drop() { return std::nullopt; }

void Animal::update(float dt) {
    wanderer.wander(*this, dt);
    advance(dt);
}
