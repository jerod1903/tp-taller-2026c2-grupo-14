#ifndef CLICK_COMMAND_H
#define CLICK_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"
#include "server/game/world/position.h"

// Un click del jugador en una celda de su área. Por ahora solo sirve para hablarle a un
// comerciante: si hay uno ahí y le llega, le manda su saludo. Si no, no pasa nada.
class ClickCommand: public GameCommand {
public:
    ClickCommand(uint32_t player_id, const ClickRequest& request);

    void execute(GameContext& context) override;

private:
    Position cell;
};

#endif  // CLICK_COMMAND_H
