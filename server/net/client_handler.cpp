#include "server/net/client_handler.h"

#include <exception>
#include <optional>
#include <stdexcept>
#include <utility>

#include "common/logger.h"

// ---- ServerReceiver ----

ServerReceiver::ServerReceiver(ServerProtocol& protocol, Queue<Action>& actions,
                               Queue<ServerMessage>& outbox, uint32_t player_id):
        protocol(protocol), actions(actions), outbox(outbox), player_id(player_id) {}

void ServerReceiver::run() {
    try {
        while (true) {
            std::optional<ClientMessage> msg = protocol.recv_game_message();  // bloquea en el socket
            if (!msg) {
                break;  // el cliente cerro
            }
            actions.push(Action{player_id, *msg});
        }
    } catch (const ClosedQueue&) {
        // la partida cerro su cola de acciones: salida normal
    } catch (const std::exception& e) {
        Logger::warn("jugador {}: conexion cortada: {}", player_id, e.what());
    }
    finish();
}

void ServerReceiver::finish() {
    // Despierta al sender dormido en pop().
    try {
        outbox.close();
    } catch (const std::runtime_error&) {}
    try {
        protocol.shutdown();
    } catch (...) {}
}

// ---- ServerSender ----

ServerSender::ServerSender(ServerProtocol& protocol, Queue<ServerMessage>& outbox,
                           uint32_t player_id):
        protocol(protocol), outbox(outbox), player_id(player_id) {}

void ServerSender::run() {
    try {
        while (true) {
            ServerMessage msg = outbox.pop();  // bloquea hasta que el Game encole algo
            if (!protocol.send_game_message(msg)) {
                break;
            }
        }
    } catch (const ClosedQueue&) {
        // la cola se cerro: salida normal
    } catch (const std::exception& e) {
        Logger::warn("jugador {}: error al enviar: {}", player_id, e.what());
    }
    finish();
}

void ServerSender::finish() {
    // Despierta al receiver dormido en recv().
    try {
        protocol.shutdown();
    } catch (...) {}
}

// ---- ClientHandler ----

ClientHandler::ClientHandler(ServerProtocol&& protocol, Queue<Action>& actions,
                             uint32_t player_id):
        protocol(std::move(protocol)),
        receiver(this->protocol, actions, out_queue, player_id),
        sender(this->protocol, out_queue, player_id) {}

ClientHandler::~ClientHandler() {
    try {
        out_queue.close();
    } catch (const std::runtime_error&) {}
    try {
        protocol.shutdown();
    } catch (...) {}
    if (started) {
        receiver.join();
        sender.join();
    }
}

void ClientHandler::start() {
    receiver.start();
    sender.start();
    started = true;
}

bool ClientHandler::is_alive() const { return receiver.is_alive() || sender.is_alive(); }

Queue<ServerMessage>& ClientHandler::outbox() { return out_queue; }