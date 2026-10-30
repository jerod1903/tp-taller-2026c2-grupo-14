#include "client/network/client_connection.h"

#include <stdexcept>


ClientSender::ClientSender(ClientProtocol& protocol, Queue<ClientMessage>& outbox):
        protocol(protocol), outbox(outbox) {}

void ClientSender::run() {
    try {
        while (true) {
            ClientMessage msg = outbox.pop();  // bloquea hasta que SDL encole algo
            if (!protocol.send(msg)) {
                break;
            }
        }
    } catch (const ClosedQueue&) {
        // la cola se cerro: salida normal
    }
    // Si el servidor corto, que send() de ClientConnection empiece a fallar.
    close_outbox();
}

// Queue::close() lanza si la cola ya estaba cerrada. Cerrarla dos veces es normal
// (el hilo la cierra al terminar y el destructor tambien), asi que se ignora.
void ClientSender::close_outbox() {
    try {
        outbox.close();
    } catch (const std::runtime_error&) {}
}


ClientReceiver::ClientReceiver(ClientProtocol& protocol, Queue<ServerMessage>& inbox):
        protocol(protocol), inbox(inbox) {}

void ClientReceiver::run() {
    try {
        while (true) {
            std::optional<ServerMessage> msg = protocol.recv();  // bloquea en el socket
            if (!msg) {
                break;  // el servidor cerro
            }
            inbox.push(*msg);
        }
    } catch (const ClosedQueue&) {
        // el destructor de ClientConnection cerro la cola
    } catch (...) {
        close_inbox();
        throw;  // Thread::main lo reporta
    }
    // Avisa a SDL: try_recv() lanza ClosedQueue cuando se vacia la cola.
    close_inbox();
}

void ClientReceiver::close_inbox() {
    try {
        inbox.close();
    } catch (const std::runtime_error&) {}
}



ClientConnection::ClientConnection(const std::string& host, const std::string& service):
        protocol(host, service), sender(protocol, outbox), receiver(protocol, inbox) {
    sender.start();
    receiver.start();
}

ClientConnection::~ClientConnection() {
    close_queues();  // despierta al sender dormido en pop()
    try {
        protocol.shutdown();  // despierta al receiver dormido en recv()
    } catch (...) {}
    sender.join();
    receiver.join();
}

void ClientConnection::close_queues() {
    try {
        outbox.close();
    } catch (const std::runtime_error&) {}
    try {
        inbox.close();
    } catch (const std::runtime_error&) {}
}

void ClientConnection::send(const ClientMessage& msg) { outbox.push(msg); }

bool ClientConnection::try_recv(ServerMessage& out) { return inbox.try_pop(out); }  