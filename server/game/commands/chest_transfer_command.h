#ifndef CHEST_TRANSFER_COMMAND_H
#define CHEST_TRANSFER_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"

// Pasa una pila entera entre el inventario del jugador y el cofre que tiene abierto. Al que la
// movió le manda su inventario; a todos los que miran el cofre, cómo quedó.
class ChestTransferCommand: public GameCommand {
public:
    ChestTransferCommand(uint32_t player_id, const ChestSlotRequest& request);
    void execute(GameContext& context) override;

private:
    ChestSlotRequest request;
};

#endif  // CHEST_TRANSFER_COMMAND_H
