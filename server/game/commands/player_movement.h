#ifndef PLAYER_MOVEMENT_H
#define PLAYER_MOVEMENT_H

#include <cstdint>

#include "server/game/world/direction.h"

// Caminar y usar puertas: lo que usan MoveCommand y EnterPortalCommand.
class PlayerMovement {
public:
    // Hacia dónde camina su personaje; NONE lo frena. Si no está en la partida, no hace nada.
    virtual void move_player(uint32_t player_id, Direction dir) = 0;
    // Si tiene una puerta al alcance, la cruza. Si no, no hace nada.
    virtual void enter_portal(uint32_t player_id) = 0;

    virtual ~PlayerMovement() = default;
};

#endif  // PLAYER_MOVEMENT_H
