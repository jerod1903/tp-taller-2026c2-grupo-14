#ifndef CLIENT_HANDLER_H
#define CLIENT_HANDLER_H

#include <cstdint>

#include "common/game_dto.h"
#include "common/lib/queue.h"
#include "common/lib/thread.h"
#include "server/game/game.h"
#include "server/net/server_protocol.h"

// Recibe del socket y deja Action{player_id, mensaje} en la cola de la partida.
// Termina cuando el cliente cierra, llegan bytes invalidos o la partida cierra su cola.
// Al terminar cierra la cola de salida y corta el socket.
class ServerReceiver: public Thread {
public:
    ServerReceiver(ServerProtocol& protocol, Queue<Action>& actions, Queue<ServerMessage>& outbox,
                   uint32_t player_id);
    void run() override;

private:
    ServerProtocol& protocol;
    Queue<Action>& actions;
    Queue<ServerMessage>& outbox;
    uint32_t player_id;

    void finish();
};

// Saca de la cola de salida del jugador y manda por el socket.
// Termina si la cola se cierra (ClosedQueue) o si send() falla. Al terminar corta el socket.
class ServerSender: public Thread {
public:
    ServerSender(ServerProtocol& protocol, Queue<ServerMessage>& outbox, uint32_t player_id);
    void run() override;

private:
    ServerProtocol& protocol;
    Queue<ServerMessage>& outbox;
    uint32_t player_id;

    void finish();
};

/*
    Todo lo del servidor para UN cliente conectado: protocolo, cola de salida
    y los dos hilos. Lo crea y lo guarda el Game.
*/
class ClientHandler {
public:
    ClientHandler(ServerProtocol&& protocol, Queue<Action>& actions, uint32_t player_id);

    // Corta la conexion, cierra la cola de salida y espera a los dos hilos.
    ~ClientHandler();

    void start();

    // true hasta que los dos hilos terminaron (para que el Game lo recolecte).
    bool is_alive() const;

    // El Game hace push aca para mandarle algo a este jugador.
    Queue<ServerMessage>& outbox();

    ClientHandler(const ClientHandler&) = delete;
    ClientHandler& operator=(const ClientHandler&) = delete;

private:
    // Solo se joinea un thread que efectivamente arranco.
    bool started = false;
    // El orden importa: receiver y sender reciben referencias a los anteriores.
    ServerProtocol protocol;
    Queue<ServerMessage> out_queue;
    ServerReceiver receiver;
    ServerSender sender;
};

#endif  // CLIENT_HANDLER_H