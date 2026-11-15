#ifndef ENVIADOR_H
#define ENVIADOR_H

#include <string>

#include "client/network/IGameProtocol.h"
#include "common/game_dto.h"

// Se encarga de mandarle al servidor las acciones del usuario.
// Si se corto la conexion, cualquier metodo tira LobbyConnectionError
class Enviador {
public:
    explicit Enviador(IGameProtocol& protocolo);
    void enviar(const MoveRequest& movimiento);
    void salir();

    void seleccionar_slot(uint8_t slot) { protocolo.sendSelectSlot(slot); }
    void enviar_chat(const std::string& texto) { protocolo.sendChat(texto); }
    void interactuar() { protocolo.sendInteract(); }

private:
    IGameProtocol& protocolo;
};

#endif
