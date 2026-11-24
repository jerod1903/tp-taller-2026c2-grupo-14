#ifndef CLOSE_CHEST_COMMAND_H
#define CLOSE_CHEST_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// El jugador cerró el panel del cofre. No se le contesta: el cliente ya lo cerró.
class CloseChestCommand: public GameCommand {
public:
    explicit CloseChestCommand(uint32_t player_id);
    void execute(GameContext& context) override;
    bool runs_while_day_ended() const override { return true; }
};

#endif  // CLOSE_CHEST_COMMAND_H
