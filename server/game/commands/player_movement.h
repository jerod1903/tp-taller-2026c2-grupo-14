#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include <cstdint>

#include "server/game/world/direction.h"

// Caminar: lo que usa MoveCommand.
class PlayerMovement {
public:
    // Hacia dónde camina su personaje; NONE lo frena. Si no está en la partida, no hace nada.
    virtual void move_player(uint32_t player_id, Direction dir) = 0;

    virtual ~PlayerMovement() = default;
};

#endif  // PLAYER_MOVEMENT_H
