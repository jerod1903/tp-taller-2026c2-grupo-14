#ifndef COMMAND_FACTORY_H
#define COMMAND_FACTORY_H

#include <cstdint>
#include <functional>
#include <map>
#include <memory>

#include "common/game_dto.h"
#include "server/game/commands/game_command.h"
#include "server/net/server_protocol.h"

// Traduce una GameOp al comando que la ejecuta, leyendo antes su payload.
class CommandFactory {
public:
    explicit CommandFactory(ServerProtocol& protocol);

    CommandFactory(const CommandFactory&) = delete;
    CommandFactory& operator=(const CommandFactory&) = delete;

    // nullptr si la op no está registrada o si el cliente cortó a mitad del payload.
    std::shared_ptr<GameCommand> create(GameOp op, uint32_t player_id);

    // nullptr si el payload no llegó completo.
    using Builder = std::function<std::shared_ptr<GameCommand>(ServerProtocol&, uint32_t)>;

private:
    ServerProtocol& protocol;
    const std::map<GameOp, Builder> builders;
};

#endif  // COMMAND_FACTORY_H
