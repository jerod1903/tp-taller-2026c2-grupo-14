#ifndef ENVIADOR_H
#define ENVIADOR_H

#include <cstdint>

#include "client/network/IGameProtocol.h"

// Se encarga de mandarle al servidor las acciones del usuario.
// Si se corto la conexion, cualquier metodo tira LobbyConnectionError
class Enviador {
public:
    explicit Enviador(IGameProtocol& protocolo);
    void mover(int8_t dir_x, int8_t dir_y);
    void salir();

private:
    IGameProtocol& protocolo;
};

#endif
