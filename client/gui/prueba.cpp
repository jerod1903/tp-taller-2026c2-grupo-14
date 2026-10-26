#include <iostream>

#include "interfaz_juego.h"

int main() {
    try {
        InterfazJuego interfaz;
        interfaz.ejecutar();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
