#ifndef SELECT_SLOT_COMMAND_H
#define SELECT_SLOT_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"

class SelectSlotCommand: public GameCommand {
public:
    SelectSlotCommand(uint32_t player_id, const SelectSlotRequest& request);
    void execute(GameContext& context) override;

private:
    SelectSlotRequest request;
};

#endif  // SELECT_SLOT_COMMAND_H