#include "enviador.h"

#include <cstdint>
#include <string>

Enviador::Enviador(IGameProtocol& protocolo): protocolo(protocolo) {}

void Enviador::enviar(const MoveRequest& movimiento) {
    protocolo.sendMove(movimiento.dirX, movimiento.dirY);
}

void Enviador::salir() {
    protocolo.sendLeaveGame();
}

void Enviador::cruzar_puerta() {
    protocolo.sendEnterPortal();
}

void Enviador::seleccionar_slot(uint8_t slot) {
    protocolo.sendSelectSlot(slot);
}

void Enviador::enviar_chat(const std::string& texto) {
    protocolo.sendChat(texto);
}

void Enviador::interactuar() {
    protocolo.sendInteract();
}

void Enviador::escribiendo(bool valor) {
    protocolo.sendTyping(valor);
}

void Enviador::clickear(const ClickRequest& click) {
    protocolo.sendClick(click.x, click.y);
}

void Enviador::invitar() {
    protocolo.sendInvite();
}

void Enviador::listo_para_manana() {
    protocolo.sendDayReady();
}

void Enviador::transferir(const ChestSlotRequest& slot) {
    protocolo.sendChestTransfer(slot.side, slot.slot);
}

void Enviador::partir(const ChestSlotRequest& slot) {
    protocolo.sendChestSplit(slot.side, slot.slot);
}

void Enviador::cerrar_cofre() {
    protocolo.sendCloseChest();
}
