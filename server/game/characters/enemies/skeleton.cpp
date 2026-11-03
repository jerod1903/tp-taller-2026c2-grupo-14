#include "server/game/characters/enemies/skeleton.h"

namespace {
// Valores provisorios hasta balancear los enemigos.
constexpr int HEALTH = 20;
constexpr float SPEED = 3.0f;
}  // namespace

Skeleton::Skeleton(uint32_t id, const Position& pos, Location& location):
        Enemy(id, pos, location, HEALTH, SPEED) {}

// TODO: comportamiento propio. Por ahora solo camina en la dirección que tenga.
void Skeleton::update(float dt) { advance(dt); }
