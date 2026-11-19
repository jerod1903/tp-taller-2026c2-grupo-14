#ifndef GAME_CONTEXT_H
#define GAME_CONTEXT_H

#include "server/game/commands/player_characters.h"
#include "server/game/commands/player_combat.h"
#include "server/game/commands/player_commerce.h"
#include "server/game/commands/player_day.h"
#include "server/game/commands/player_farming.h"
#include "server/game/commands/player_movement.h"
#include "server/game/commands/player_sessions.h"

// Lo que los comandos le pueden pedir a la partida, separado por tema: cada comando usa solo la
// parte que necesita. Una acción nueva va en la interfaz de su tema; un tema nuevo suma aquí un
// método que la devuelva.
class GameContext {
public:
    virtual PlayerSessions& sessions() = 0;
    virtual PlayerMovement& movement() = 0;
    virtual PlayerCharacters& characters() = 0;
    virtual PlayerCommerce& commerce() = 0;
    virtual PlayerFarming& farming() = 0;
    virtual PlayerCombat& combat() = 0;
    virtual PlayerDay& day() = 0;

    virtual ~GameContext() = default;
};

#endif  // GAME_CONTEXT_H
