#ifndef RECIBIDOR_H
#define RECIBIDOR_H

#include <optional>
#include <vector>

#include "common/estado_personaje.h"
// #include "protocolo_cliente.h"

class Recibidor {
public:
    // explicit Recibidor(ProtocoloCliente& protocolo);
    std::optional<std::vector<EstadoPersonaje>> recibir();

private:
    // ProtocoloCliente& protocolo;
};

#endif
