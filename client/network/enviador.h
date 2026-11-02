#ifndef ENVIADOR_H
#define ENVIADOR_H

#include "common/requests.h"
// TODO: descomentar cuando exista el protocolo del cliente
// #include "protocolo_cliente.h"

// Se encarga de mandarle al servidor las acciones del usuario.
class Enviador {
public:
    // TODO: descomentar cuando exista el protocolo del cliente
    // explicit Enviador(ProtocoloCliente& protocolo);
    void enviar(const Request& request);

private:
    // ProtocoloCliente& protocolo;
};

#endif
