#include <exception>
#include <iostream>

#include "client/app/ClientApp.h"
#include "client/client.h"
#include "client/menu/QtMenu.h"
#include "client/network/ClientProtocol.h"
#include "common/net/socket.h"

namespace {
constexpr int EXPECTED_ARGC = 3;
constexpr int ARG_HOST = 1;
constexpr int ARG_PORT = 2;
}  // namespace

// Uso: taller_client <host> <puerto>
int main(int argc, char* argv[]) {
    if (argc != EXPECTED_ARGC) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto>" << std::endl;
        return 1;
    }
    try {
        // Una sola conexion para el lobby y las partidas: el menu y el juego la comparten.
        ClientProtocol protocol(Socket(argv[ARG_HOST], argv[ARG_PORT]));
        QtMenu menu(argc, argv, protocol);
        Cliente game(protocol);
        ClientApp(menu, game).run();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
