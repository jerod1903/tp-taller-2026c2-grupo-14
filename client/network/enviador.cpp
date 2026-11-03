#include "enviador.h"

Enviador::Enviador(ClientProtocol& protocolo): protocolo(protocolo) {}

void Enviador::enviar(const Request& request) { protocolo.sendRequest(request); }
