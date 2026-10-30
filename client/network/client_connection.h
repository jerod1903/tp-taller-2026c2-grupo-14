#ifndef CLIENT_CONNECTION_H
#define CLIENT_CONNECTION_H

#include <string>

#include "client/network/client_protocol.h"
#include "common/game_dto.h"
#include "common/lib/queue.h"
#include "common/lib/thread.h"




// Saca de la cola de envio y manda por el socket. Termina si la cola se cierra
// (ClosedQueue) o si send() devuelve false.
class ClientSender: public Thread {
public:
    ClientSender(ClientProtocol& protocol, Queue<ClientMessage>& outbox);
    void run() override;

private:
    ClientProtocol& protocol;
    Queue<ClientMessage>& outbox;
    
    void close_outbox();
};

// Recibe del socket y deja en la cola de recepcion. Al terminar (nullopt)
// cierra cola de recepcion para avisarle a SDL.
class ClientReceiver: public Thread {
public:
    ClientReceiver(ClientProtocol& protocol, Queue<ServerMessage>& inbox);
    void run() override;

    void close_inbox();

private:
    ClientProtocol& protocol;
    Queue<ServerMessage>& inbox;
};

/*
    Lo unico que ve el cliente SDL de la red. Nada de lo que hay aca bloquea
    al hilo principal.
*/
class ClientConnection {
public:
    // Se conecta y lanza los dos hilos. Lanza una excepcion si no puede conectarse.
    ClientConnection(const std::string& host, const std::string& service);

    // Cierra las colas, hace shutdown del socket y espera a los hilos.
    ~ClientConnection();

    // Encola un mensaje para enviar. Lanza ClosedQueue si la conexion ya se cerro.
    void send(const ClientMessage& msg);

    // Saca un mensaje recibido si hay alguno. false si no hay nada por ahora.
    // Lanza ClosedQueue si el servidor cerro la conexion y no quedan mensajes.
    bool try_recv(ServerMessage& out);

    ClientConnection(const ClientConnection&) = delete;
    ClientConnection& operator=(const ClientConnection&) = delete;

private:
    // El orden importa: sender y receiver reciben referencias a los anteriores.
    ClientProtocol protocol;
    Queue<ClientMessage> outbox;
    Queue<ServerMessage> inbox;
    ClientSender sender;
    ClientReceiver receiver;


    void close_queues();
};

#endif  // CLIENT_CONNECTION_H