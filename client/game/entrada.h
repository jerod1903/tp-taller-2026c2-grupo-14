#ifndef ENTRADA_H
#define ENTRADA_H

#include <cstdint>
#include <optional>

// Lo que hizo el usuario en un frame, sin depender de SDL.
struct Entrada {
    bool salir = false;  // cerro la ventana o apreto Escape
    int8_t dir_x = 0;    // -1 izq, 0 no se mueve, 1 der
    int8_t dir_y = 0;    // -1 arriba, 0 no se mueve, 1 abajo
    std::optional<uint8_t> slot_seleccionado;  // 0..8 si apretó teclas 1..9
};

#endif
