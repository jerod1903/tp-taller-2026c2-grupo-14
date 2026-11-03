#ifndef SKELETON_H
#define SKELETON_H

#include <cstdint>

#include "server/game/characters/enemies/enemy.h"

class Skeleton: public Enemy {
public:
    Skeleton(uint32_t id, const Position& pos, Location& location);

    void update(float dt) override;
};

#endif  // SKELETON_H
