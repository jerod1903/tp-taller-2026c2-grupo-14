#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "server/game/commands/player_movement.h"
#include "server/game/commands/player_sessions.h"

// Lo que los comandos le pueden pedir a la partida, separado por tema: cada comando usa solo la
// parte que necesita. Una acción nueva va en la interfaz de su tema; un tema nuevo suma aquí un
// método que la devuelva.
class GameContext {
public:
    virtual PlayerSessions& sessions() = 0;
    virtual PlayerMovement& movement() = 0;

    virtual ~GameContext() = default;
};

#endif  // GAME_CONTEXT_H
