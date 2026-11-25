#ifndef ENVIADOR_H
#define ENVIADOR_H

#include <cstdint>
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
    void cruzar_puerta();
    void seleccionar_slot(uint8_t slot);
    void enviar_chat(const std::string& texto);
    void interactuar();
    void escribiendo(bool valor);
    void clickear(const ClickRequest& click);
    void invitar();
    void listo_para_manana();
    void transferir(const ChestSlotRequest& slot);
    void partir(const ChestSlotRequest& slot);
    void cerrar_cofre();

private:
    IGameProtocol& protocolo;
};

#endif
