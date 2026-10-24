#include "armador_acciones.h"
#include "constants.h"

std::vector<uint8_t> ArmadorAcciones::mover(int dir_x, int dir_y) const {
    return {ACCION_MOVER, codigo_direccion(dir_x, dir_y)};
}

uint8_t ArmadorAcciones::codigo_direccion(int dir_x, int dir_y) const {
    if (dir_y < 0) {
        if (dir_x < 0) {
            return DIRECCION_ARRIBA_IZQUIERDA;
        }
        if (dir_x > 0) {
            return DIRECCION_ARRIBA_DERECHA;
        }
        return DIRECCION_ARRIBA;
    }
    if (dir_y > 0) {
        if (dir_x < 0) {
            return DIRECCION_ABAJO_IZQUIERDA;
        }
        if (dir_x > 0) {
            return DIRECCION_ABAJO_DERECHA;
        }
        return DIRECCION_ABAJO;
    }
    if (dir_x < 0) {
        return DIRECCION_IZQUIERDA;
    }
    return DIRECCION_DERECHA;
}
