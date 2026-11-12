#ifndef MOVE_COMMAND_H
#define MOVE_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"

// Fija hacia dónde camina el personaje; (0, 0) lo frena.
class MoveCommand: public GameCommand {
public:
    MoveCommand(uint32_t player_id, const MoveRequest& request);

    void execute(GameContext& context) override;

private:
    const MoveRequest request;
};

#endif  // MOVE_COMMAND_H
