#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <cstdint>

#include "common/game_dto.h"
#include "common/lib/queue.h"
#include "common/lib/thread.h"
#include "common/net/socket.h"
#include "server/game_manager.h"
#include "server/server_protocol.h"

// Recibe del socket y deja Action{player_id, mensaje} en la cola de la partida.
// Termina cuando el cliente cierra o llegan bytes invalidos.
class ServerReceiver: public Thread {
public:
    ServerReceiver(ServerProtocol& protocol, Queue<Action>& actions, uint32_t player_id);
    void run() override;

private:
    ServerProtocol& protocol;
    Queue<Action>& actions;
    uint32_t player_id;
};

// Saca de la cola de salida del jugador y manda por el socket.
// Termina si la cola se cierra (ClosedQueue) o si send() devuelve false.
class ServerSender: public Thread {
public:
    ServerSender(ServerProtocol& protocol, Queue<ServerMessage>& outbox);
    void run() override;

private:
    ServerProtocol& protocol;
    Queue<ServerMessage>& outbox;
};

/*
    Todo lo del servidor para UN cliente conectado: protocolo, cola de salida
    y los dos hilos. Al construirse registra al jugador en la partida.
    Al terminar cualquiera de los dos hilos, saca al jugador de la partida,
    cierra la cola de salida y hace shutdown del socket.
*/
class ClientHandler {
public:
    ClientHandler(Socket&& socket, Game& game);

    void start();
    void stop();
    void join();

    // true hasta que los dos hilos terminaron (para que el Acceptor lo recolecte).
    bool is_alive() const;

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

private:
    ServerProtocol protocol;
    Queue<ServerMessage> outbox;
    Game& game;
    uint32_t player_id;
    ServerReceiver receiver;
    ServerSender sender;
};

#endif  // CLIENT_HANDLER_H