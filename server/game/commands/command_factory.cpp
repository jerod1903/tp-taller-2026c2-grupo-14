#include "server/game/commands/command_factory.h"
#include "server/game/commands/select_slot_command.h"
#include <optional>

#include "server/game/commands/chat_command.h"
#include "server/game/commands/move_command.h"

namespace {

// Payload se deduce de `recv`: with_payload<MoveCommand>(&ServerProtocol::recv_move).
template <typename Command, typename Payload>
CommandFactory::Builder with_payload(std::optional<Payload> (ServerProtocol::*recv)()) {
    return [recv](ServerProtocol& protocol, uint32_t player_id) -> std::shared_ptr<GameCommand> {
        std::optional<Payload> payload = (protocol.*recv)();
        if (!payload.has_value()) {
            return nullptr;
        }
        return std::make_shared<Command>(player_id, *payload);
    };
}

template <typename Command>
CommandFactory::Builder without_payload() {
    return [](ServerProtocol&, uint32_t player_id) -> std::shared_ptr<GameCommand> {
        return std::make_shared<Command>(player_id);
    };
}

}  // namespace

// LeaveGame no se registra: la resuelve el Receiver, que necesita saber que el jugador salió.
CommandFactory::CommandFactory(ServerProtocol& protocol):
        protocol(protocol),
        builders{
                {GameOp::Move, with_payload<MoveCommand>(&ServerProtocol::recv_move)},
                {GameOp::SelectSlot, with_payload<SelectSlotCommand>(&ServerProtocol::recv_select_slot)},
                {GameOp::Chat, with_payload<ChatCommand>(&ServerProtocol::recv_chat)},
        } {}

std::shared_ptr<GameCommand> CommandFactory::create(GameOp op, uint32_t player_id) {
    auto it = builders.find(op);
    if (it != builders.end()) {
        return it->second(protocol, player_id);
    }
    return nullptr;
}
