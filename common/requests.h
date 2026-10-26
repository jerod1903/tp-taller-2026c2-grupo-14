#ifndef REQUESTS_H
#define REQUESTS_H

#include <cstdint>

#include "operations.h"

struct Request {
    Operation tipo;
    uint8_t dir_x;  // 0 si no se mueve, 1 der, 2 izq
    uint8_t dir_y;  // 0 si no se mueve, 1 arriba, 2 abajo
};

#endif
