#pragma once

#include <cstdint>
#include <string>
#include <variant>

#include "common/game_dto.h"

// Lo que manda el servidor durante la partida.
using ServerGameMessage =
        std::variant<LocationData, Snapshot, GameEnded, PlayerInfo, InventoryState, ChatMessage,
                     FarmState, DungeonState, DayEnded, ChestState, GameCompleted>;


// Lo que la partida necesita del servidor: un send por cada accion del jugador y el recv de lo que
// manda el servidor. Los send tiran LobbyConnectionError si se corto la conexion.
class IGameProtocol {
public:
    virtual ~IGameProtocol() = default;

    // dir_x: -1 izq, 0 quieto, 1 der. dir_y: -1 arriba, 0 quieto, 1 abajo. (0, 0) frena
    virtual void sendMove(int8_t dir_x, int8_t dir_y) = 0;
    virtual void sendLeaveGame() = 0;
    // Cruza la puerta que el jugador tiene al alcance, si hay una
    virtual void sendEnterPortal() = 0;
    // Bloquea hasta que llega un mensaje. Si el tag no se conoce tira std::runtime_error.
    virtual ServerGameMessage recvGameMessage() = 0;
    // Corta la conexion: despierta a quien este bloqueado en recvGameMessage
    virtual void shutdown() = 0;

    virtual void sendSelectSlot(uint8_t slot) = 0;
    virtual void sendChat(const std::string& texto) = 0;
    virtual void sendInteract() = 0;
    virtual void sendTyping(bool escribiendo) = 0;
    virtual void sendClick(uint16_t x, uint16_t y) = 0;
    virtual void sendInvite() = 0;
    // Da el Ok para pasar al día siguiente
    virtual void sendDayReady() = 0;
    // Con el panel del cofre abierto: pasa esa pila entera al otro inventario
    virtual void sendChestTransfer(ChestSide side, uint8_t slot) = 0;
    // Con el panel del cofre abierto: pasa la mitad de esa pila al primer slot vacío
    virtual void sendChestSplit(ChestSide side, uint8_t slot) = 0;
    virtual void sendCloseChest() = 0;
};
