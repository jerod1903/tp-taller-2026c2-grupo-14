#include "server/net/receiver.h"

#include <optional>

#include "common/game_dto.h"
#include "common/logger.h"
#include "common/net/liberror.h"
#include "common/operations.h"

Receiver::Receiver(ServerProtocol& protocol, Inbox& inbox, uint32_t player_id):
        protocol(protocol), inbox(inbox), player_id(player_id) {}

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
            if (*op == GameOp::Move) {
                std::optional<MoveRequest> move = protocol.recv_move();
                if (!move) {
                    break;
                }
                inbox.push(
                        PlayerAction{player_id, Request{Operation::Move, move->dirX, move->dirY}});
            } else {
                break;  // Op desconocida: cortar la conexión
            }
        }
    } catch (const LibError& e) {
        Logger::debug("conexión cortada: {}", e.what());
    } catch (const ClosedQueue&) {
        return;  // El Game se cerró: no hay a quién avisar.
    }
    try {
        inbox.push(PlayerLeft{player_id});
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
