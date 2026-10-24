#ifndef ARMADOR_ACCIONES_H
#define ARMADOR_ACCIONES_H

#include <cstdint>
#include <vector>

// Arma la tira de bytes de cada accion del usuario, lista para mandar al
// servidor. Todos los bytes son u8.
//
// Mover (2 bytes): [ACCION_MOVER][DIRECCION_...]
class ArmadorAcciones {
public:
    // dir_x: -1 izquierda, 0, 1 derecha. dir_y: -1 arriba, 0, 1 abajo.
    // No llamar con las dos en 0.
    std::vector<uint8_t> mover(int dir_x, int dir_y) const;

private:
    uint8_t codigo_direccion(int dir_x, int dir_y) const;
};

#endif
