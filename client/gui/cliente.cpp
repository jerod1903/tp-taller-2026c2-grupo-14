#include "cliente.h"

#include "client/network/enviador.h"
#include "client/network/recibidor.h"

#include "interfaz.h"

Cliente::Cliente(ClientProtocol& protocolo, uint32_t id_propio):
        protocolo(protocolo), id_propio(id_propio) {}

void Cliente::ejecutar() {
    Enviador enviador(protocolo);
    Recibidor recibidor(protocolo);
    Interfaz interfaz(enviador, recibidor, id_propio);
    interfaz.ejecutar();
}
