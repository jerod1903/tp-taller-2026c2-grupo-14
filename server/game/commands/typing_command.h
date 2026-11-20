#ifndef TYPING_COMMAND_H
#define TYPING_COMMAND_H

#include <cstdint>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"

// El jugador abrio o cerro el chat. Solo cambia su estado: el globito de "escribiendo" lo ven
// todos en el proximo snapshot, como Emote::Typing de su entidad.
class TypingCommand: public GameCommand {
public:
    TypingCommand(uint32_t player_id, const TypingRequest& request);
    void execute(GameContext& context) override;
    bool runs_while_day_ended() const override { return true; }

private:
    bool typing;
};

#endif  // TYPING_COMMAND_H
