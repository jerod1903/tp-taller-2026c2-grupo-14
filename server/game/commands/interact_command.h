#ifndef INTERACT_COMMAND_H
#define INTERACT_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// El jugador apretó la tecla de interactuar: se hace la primera de interactions() que tenga con
// qué hacerse al alcance. Si se hizo, cansa y le manda el inventario nuevo; si no le entraba en
// el inventario, le avisa por el chat. Si no hay ninguna al alcance, no pasa nada.
class InteractCommand: public GameCommand {
public:
    explicit InteractCommand(uint32_t player_id);
    void execute(GameContext& context) override;
};

#endif  // INTERACT_COMMAND_H
