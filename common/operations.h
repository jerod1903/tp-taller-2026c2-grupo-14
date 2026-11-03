#ifndef OPERATIONS_H
#define OPERATIONS_H

#include <cstdint>

enum class Operation : uint8_t {
    Move = 0x01,
    LeaveGame = 0x02,  // sin dirección: dir_x y dir_y van en 0
};

#endif
