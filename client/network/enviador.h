#ifndef ENVIADOR_H
#define ENVIADOR_H

#include "client/network/ClientProtocol.h"
#include "common/requests.h"

// Se encarga de mandarle al servidor las acciones del usuario.
class Enviador {
public:
    explicit Enviador(ClientProtocol& protocolo);
    // Si se corto la conexion tira LobbyConnectionError
    void enviar(const Request& request);

private:
    ClientProtocol& protocolo;
};

#endif
