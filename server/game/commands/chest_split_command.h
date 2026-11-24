#ifndef CHEST_SPLIT_COMMAND_H
#define CHEST_SPLIT_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"

// Parte una pila del inventario del jugador o del cofre que tiene abierto. Si partió la suya le
// manda su inventario; si partió la del cofre, el cofre a todos los que lo miran.
class ChestSplitCommand: public GameCommand {
public:
    ChestSplitCommand(uint32_t player_id, const ChestSlotRequest& request);
    void execute(GameContext& context) override;

private:
    ChestSlotRequest request;
};

#endif  // CHEST_SPLIT_COMMAND_H
