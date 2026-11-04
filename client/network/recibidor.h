#ifndef RECIBIDOR_H
#define RECIBIDOR_H

#include <optional>

#include "client/menu/backend/ILobbyProtocol.h"
#include "client/network/IGameProtocol.h"
#include "common/lib/queue.h"
#include "common/lib/thread.h"

// Hilo que recibe lo que manda el servidor durante la partida y lo deja en una cola: el recv
// bloquea, y si lo hiciera la interfaz se congelaria la ventana. Arranca al construirse y termina
// cuando llega GameEnded o se corta la conexion.
class Recibidor: public Thread {
public:
    explicit Recibidor(IGameProtocol& protocolo);
    void run() override;
    // No bloquea: nullopt si no llego nada nuevo
    std::optional<ServerGameMessage> recibir();
    // Si el hilo sigue esperando un mensaje, corta la conexion para despertarlo
    ~Recibidor() override;

private:
    IGameProtocol& protocolo;
    Queue<ServerGameMessage> mensajes;
};

#endif
