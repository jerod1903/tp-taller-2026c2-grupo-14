#ifndef DAY_READY_COMMAND_H
#define DAY_READY_COMMAND_H

#include <cstdint>

#include "server/game/commands/game_command.h"

// El jugador dio el Ok para pasar al día siguiente.
class DayReadyCommand: public GameCommand {
public:
    explicit DayReadyCommand(uint32_t player_id);
    void execute(GameContext& context) override;
    bool runs_while_day_ended() const override;
};

#endif  // DAY_READY_COMMAND_H
