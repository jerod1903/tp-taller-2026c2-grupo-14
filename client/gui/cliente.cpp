#include "cliente.h"

#include "client/network/enviador.h"
#include "client/network/recibidor.h"

#include "interfaz.h"

void Cliente::ejecutar() {
    // TODO: cuando exista el protocolo del cliente:
    // ProtocoloCliente protocolo(host, puerto);
    // Enviador enviador(protocolo);
    // Recibidor recibidor(protocolo);
    Enviador enviador;
    Recibidor recibidor;
    Interfaz interfaz(enviador, recibidor);
    interfaz.ejecutar();
}
