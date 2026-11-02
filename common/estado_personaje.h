#ifndef ESTADO_PERSONAJE_H
#define ESTADO_PERSONAJE_H

#include <cstdint>

// Lo que el servidor manda de cada jugador en cada tick.
struct EstadoPersonaje {
    uint16_t id;
    int x;  // esquina superior izquierda del sprite, en pixeles del tileset (sin escalar)
    int y;
};

#endif
