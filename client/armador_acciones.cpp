#include "armador_acciones.h"
#include "constants.h"
#include <map>
#include <utility>

std::vector<uint8_t> ArmadorAcciones::mover(int dir_x, int dir_y) const {
    return {ACCION_MOVER, codigo_direccion(dir_x, dir_y)};
}

uint8_t ArmadorAcciones::codigo_direccion(int dir_x, int dir_y) const {
    static std::map<std::pair<int, int>, uint8_t> direcciones = {
        {{0, -1}, DIRECCION_ARRIBA},
        {{0, 1}, DIRECCION_ABAJO},
        {{-1, 0}, DIRECCION_IZQUIERDA},
        {{1, 0}, DIRECCION_DERECHA},
        {{-1, -1}, DIRECCION_ARRIBA_IZQUIERDA},
        {{1, -1}, DIRECCION_ARRIBA_DERECHA},
        {{-1, 1}, DIRECCION_ABAJO_IZQUIERDA},
        {{1, 1}, DIRECCION_ABAJO_DERECHA}
    };
    return direcciones.at({dir_x, dir_y});
}
