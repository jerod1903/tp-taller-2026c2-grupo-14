#include "enviador.h"

Enviador::Enviador(IGameProtocol& protocolo): protocolo(protocolo) {}

void Enviador::mover(int8_t dir_x, int8_t dir_y) { protocolo.sendMove(dir_x, dir_y); }

void Enviador::salir() { protocolo.sendLeaveGame(); }
