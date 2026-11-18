#ifndef CLICK_COMMAND_H
#define CLICK_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"
#include "server/game/world/position.h"

// El jugador clickeó una celda: hace la primera ClickAction que aplique ahí.
class ClickCommand: public GameCommand {
public:
    ClickCommand(uint32_t player_id, const ClickRequest& click);
    void execute(GameContext& context) override;

private:
    const Position cell;
};

#endif  // CLICK_COMMAND_H
