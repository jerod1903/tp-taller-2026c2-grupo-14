#ifndef GAME_COMMAND_H
#define GAME_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_context.h"

// Acción de un jugador sobre el estado de la partida. Se ejecuta en el hilo del Game.
class GameCommand {
public:
    explicit GameCommand(uint32_t player_id): player_id(player_id) {}

    virtual void execute(GameContext& context) = 0;

    // Si se ejecuta mientras se vota pasar de día, con el mundo congelado. Por defecto no: las
    // acciones sobre el mundo se descartan. Sí lo hacen entrar, salir, el chat y el Ok.
    virtual bool runs_while_day_ended() const { return false; }

    virtual ~GameCommand() = default;

protected:
    const uint32_t player_id;
};

#endif  // GAME_COMMAND_H
