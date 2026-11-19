#ifndef JOIN_COMMAND_H
#define JOIN_COMMAND_H

#include <cstdint>
#include <memory>

#include "server/game/commands/game_command.h"

// El jugador entra a la partida: lo encola el lobby, no viene del cliente.
class JoinCommand: public GameCommand {
public:
    JoinCommand(uint32_t player_id, GameMember member, std::shared_ptr<Outbox> outbox);

    void execute(GameContext& context) override;
    bool runs_while_day_ended() const override { return true; }

private:
    const GameMember member;
    const std::shared_ptr<Outbox> outbox;
};

#endif  // JOIN_COMMAND_H
