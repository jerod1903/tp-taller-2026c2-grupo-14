#ifndef INTERACT_COMMAND_H
#define INTERACT_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// El jugador apreto la tecla de interactuar. Por ahora solo sirve para ordeñar: busca la vaca
// lista mas cercana en su misma area, a lo sumo a MILKING_REACH casillas (de centro a centro).
// Si la encuentra, le suma una leche al inventario, deja a la vaca sin leche y le manda el
// inventario nuevo. Si el inventario esta lleno, le avisa por el chat y la vaca sigue lista.
// Si no hay ninguna vaca lista cerca, no pasa nada.
class InteractCommand: public GameCommand {
public:
    explicit InteractCommand(uint32_t player_id);
    void execute(GameContext& context) override;
};

#endif  // INTERACT_COMMAND_H
