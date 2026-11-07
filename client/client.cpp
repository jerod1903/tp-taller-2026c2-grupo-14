#include "client/client.h"

#include "client/game/game_loop.h"
#include "client/gui/interfaz.h"
#include "client/network/enviador.h"
#include "client/network/recibidor.h"

Cliente::Cliente(IGameProtocol& protocolo, uint32_t id_propio):
        protocolo(protocolo), id_propio(id_propio) {}

void Cliente::ejecutar() {
    Enviador enviador(protocolo);
    Recibidor recibidor(protocolo);
    Interfaz interfaz;
    GameLoop game_loop(enviador, recibidor, interfaz, id_propio);
    game_loop.ejecutar();
}
