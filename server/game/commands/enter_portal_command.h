#ifndef ENTER_PORTAL_COMMAND_H
#define ENTER_PORTAL_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// Cruza la puerta que el personaje tiene al alcance.
class EnterPortalCommand: public GameCommand {
public:
    explicit EnterPortalCommand(uint32_t player_id);

    void execute(GameContext& context) override;
};

#endif  // ENTER_PORTAL_COMMAND_H
