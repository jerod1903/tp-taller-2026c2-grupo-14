#include "interfaz_juego.h"
#include <iostream>

int main() {
    try {
        InterfazJuego interfaz;
        interfaz.ejecutar();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
};
