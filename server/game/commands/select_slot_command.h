#ifndef SELECT_SLOT_COMMAND_H
#define SELECT_SLOT_COMMAND_H

#include <cstdint>
#include "server/game/commands/game_command.h"
#include "common/game_dto.h"

class SelectSlotCommand : public GameCommand {
public:
    SelectSlotCommand(uint32_t player_id, const SelectSlotRequest& request);
    void execute(GameContext& context) override;

private:
    uint8_t slot;
};

#endif  // SELECT_SLOT_COMMAND_H