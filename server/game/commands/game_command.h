#ifndef GAME_COMMAND_H
#define GAME_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_context.h"

// Acción de un jugador sobre el estado de la partida. Se ejecuta en el hilo del Game.
class GameCommand {
public:
    explicit GameCommand(uint32_t player_id): player_id(player_id) {}

    virtual void execute(GameContext& context) = 0;

    virtual ~GameCommand() = default;

protected:
    const uint32_t player_id;
};

#endif  // GAME_COMMAND_H
