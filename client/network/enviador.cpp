#include "enviador.h"

Enviador::Enviador(IGameProtocol& protocolo): protocolo(protocolo) {}

void Enviador::enviar(const MoveRequest& movimiento) {
    protocolo.sendMove(movimiento.dirX, movimiento.dirY);
}

void Enviador::salir() { protocolo.sendLeaveGame(); }
