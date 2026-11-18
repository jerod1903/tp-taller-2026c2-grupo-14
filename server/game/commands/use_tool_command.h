#ifndef USE_TOOL_COMMAND_H
#define USE_TOOL_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// El jugador usó lo que tiene en el slot elegido sobre la celda que tiene adelante.
class UseToolCommand: public GameCommand {
public:
    explicit UseToolCommand(uint32_t player_id);
    void execute(GameContext& context) override;
};

#endif  // USE_TOOL_COMMAND_H
