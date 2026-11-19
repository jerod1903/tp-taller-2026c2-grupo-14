#ifndef LEAVE_COMMAND_H
#define LEAVE_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// El jugador sale de la partida: pidió LeaveGame o se cortó su conexión.
class LeaveCommand: public GameCommand {
public:
    explicit LeaveCommand(uint32_t player_id);

    void execute(GameContext& context) override;
    bool runs_while_day_ended() const override { return true; }
};

#endif  // LEAVE_COMMAND_H
