#ifndef REQUESTS_H
#define REQUESTS_H

#include <cstdint>

#include "operations.h"

struct Request {
    Operation tipo;
    int8_t dir_x;  // -1 izq, 0 no se mueve, 1 der
    int8_t dir_y;  // -1 arriba, 0 no se mueve, 1 abajo (en pantalla la y crece hacia abajo)
};

#endif
