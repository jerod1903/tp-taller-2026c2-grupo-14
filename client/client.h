#ifndef CLIENT_H
#define CLIENT_H

#include <cstdint>

#include "client/network/IGameProtocol.h"

class Cliente {
public:
    // protocolo: la conexion ya dentro de la partida (despues de entrar desde el lobby)
    // id_propio: el playerId que devolvio el servidor al entrar
    Cliente(IGameProtocol& protocolo, uint32_t id_propio);
    void ejecutar();

private:
    IGameProtocol& protocolo;
    uint32_t id_propio;
};

#endif
