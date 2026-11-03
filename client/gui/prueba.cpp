#include <iostream>

#include "cliente.h"

int main() {
    try {
        Cliente cliente;
        cliente.ejecutar();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
