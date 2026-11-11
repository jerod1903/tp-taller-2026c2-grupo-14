#ifndef SONIDO_H
#define SONIDO_H

#include <cstdint>

// Los efectos de sonido del juego. Cada uno tiene su archivo en RUTAS_SONIDOS (gui/rutas.h)
enum class Sonido : uint8_t {
    Plantar,
    Regar,
    Cosechar,
    Azada,
    Recoger,
    Hacha,
    Pico,
    LanzarCana,
    Boya,
    AtraparPez,
    Paso1,
    Paso2,
    Paso3,
    Paso4,
    Moneda,
    NuevoDia,
    Gallina,
};

#endif
