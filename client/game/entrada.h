#ifndef ENTRADA_H
#define ENTRADA_H

#include <cstdint>
#include <optional>
#include <string>

struct Entrada {
    bool salir = false;
    int8_t dir_x = 0;
    int8_t dir_y = 0;
    std::optional<uint8_t> slot_seleccionado;
    std::optional<std::string> mensaje_chat;
    bool interactuar = false;
    std::optional<bool> escribiendo;
};

#endif
