#ifndef ENTRADA_H
#define ENTRADA_H

#include <cstdint>
#include <optional>
#include <string>

#include "common/game_dto.h"

// Lo que hizo el usuario en un frame, sin depender de SDL.
struct Entrada {
    bool salir = false;          // cerro la ventana o apreto Escape
    bool cruzar_puerta = false;  // apreto X: entra o sale por la puerta que tiene al lado
    int8_t dir_x = 0;            // -1 izq, 0 no se mueve, 1 der
    int8_t dir_y = 0;            // -1 arriba, 0 no se mueve, 1 abajo
    std::optional<uint8_t> slot_seleccionado;
    std::optional<std::string> mensaje_chat;
    bool interactuar = false;
    std::optional<bool> escribiendo;
    std::optional<ClickRequest> click;
    bool invitar = false;
    bool listo_para_manana = false;  // dio el Ok en la pantalla de fin de día
    // Con el panel del cofre abierto: click izquierdo pasa la pila, derecho la parte
    std::optional<ChestSlotRequest> transferencia;
    std::optional<ChestSlotRequest> particion;
    bool cerrar_cofre = false;
    bool cerrar_juego_completado = false;  // Seguir jugando en la felicitación
};

#endif
