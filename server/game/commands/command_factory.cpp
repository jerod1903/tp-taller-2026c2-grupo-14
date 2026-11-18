#include "server/game/commands/command_factory.h"

#include <memory>
#include <optional>

#include "server/game/commands/chat_command.h"
#include "server/game/commands/click_command.h"
#include "server/game/commands/enter_portal_command.h"
#include "server/game/commands/interact_command.h"
#include "server/game/commands/move_command.h"
#include "server/game/commands/select_slot_command.h"
#include "server/game/commands/typing_command.h"

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
                {GameOp::EnterPortal, without_payload<EnterPortalCommand>()},
                {GameOp::SelectSlot,
                 with_payload<SelectSlotCommand>(&ServerProtocol::recv_select_slot)},
                {GameOp::Chat, with_payload<ChatCommand>(&ServerProtocol::recv_chat)},
                {GameOp::Interact, without_payload<InteractCommand>()},
                {GameOp::Typing, with_payload<TypingCommand>(&ServerProtocol::recv_typing)},
                {GameOp::Click, with_payload<ClickCommand>(&ServerProtocol::recv_click)},
        } {}

std::shared_ptr<GameCommand> CommandFactory::create(GameOp op, uint32_t player_id) {
    auto it = builders.find(op);
    if (it != builders.end()) {
        return it->second(protocol, player_id);
    }
    return nullptr;
}
