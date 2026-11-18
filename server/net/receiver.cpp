#include "server/net/receiver.h"

#include <memory>
#include <optional>
#include <utility>

#include "common/game_dto.h"
#include "common/logger.h"
#include "common/net/liberror.h"
#include "server/game/commands/leave_command.h"

Receiver::Receiver(ServerProtocol& protocol, Inbox& inbox, uint32_t player_id,
                   std::function<void()> on_invite):
        protocol(protocol),
        factory(protocol),
        inbox(inbox),
        player_id(player_id),
        on_invite(std::move(on_invite)) {}

void Receiver::start() {
    Thread::start();
    started = true;
}

void Receiver::run() {
    try {
        while (should_keep_running()) {
            std::optional<GameOp> op = protocol.recv_game_op();
            if (!op) {
                break;
            }
            if (*op == GameOp::LeaveGame) {
                left = true;
                break;
            }
            if (*op == GameOp::Invite) {
                on_invite();
                continue;
            }
            std::shared_ptr<GameCommand> command = factory.create(*op, player_id);
            if (!command) {
                // No se sabe dónde empieza la próxima op: se corta como si fuera una desconexión.
                Logger::debug("op desconocida o payload cortado: {}", static_cast<int>(*op));
                break;
            }
            inbox.push(command);
        }
    } catch (const LibError& e) {
        Logger::debug("conexión cortada: {}", e.what());
    } catch (const ClosedQueue&) {
        return;  // El Game se cerró: no hay a quién avisar.
    }
    try {
        inbox.push(std::make_shared<LeaveCommand>(player_id));
    } catch (const ClosedQueue&) {}
}

void Receiver::join() {
    Thread::join();
    started = false;  // El destructor no lo vuelve a joinear.
}

bool Receiver::has_left() const { return left; }

Receiver::~Receiver() {
    stop();
    if (started) {
        join();
    }
}
