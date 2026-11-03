#include <iostream>

#include "client/network/ClientProtocol.h"
#include "common/net/socket.h"

#include "cliente.h"

// Provisorio, hasta que el menu Qt le pase la conexion al juego: entra directo a una partida nueva
// en el mundo "valle". Uso: ./juego <host> <puerto> <usuario>
int main(int argc, char* argv[]) {
    if (argc != 4) {
        std::cerr << "Uso: " << argv[0] << " <host> <puerto> <usuario>" << std::endl;
        return 1;
    }
    try {
        ClientProtocol protocolo(Socket(argv[1], argv[2]));
        if (protocolo.login(argv[3]) != LobbyStatus::Ok) {
            std::cerr << "El servidor rechazo el login" << std::endl;
            return 1;
        }
        CreateGameRequest pedido{"valle", "prueba", argv[3], {0, 0, 0, 0, 0}};
        GameEnteredResponse partida = protocolo.createGame(pedido);
        if (partida.status != LobbyStatus::Ok) {
            std::cerr << "No se pudo crear la partida" << std::endl;
            return 1;
        }
        Cliente cliente(protocolo, partida.playerId);
        cliente.ejecutar();
    } catch (const std::exception& e) {
        std::cerr << e.what() << std::endl;
        return 1;
    }
    return 0;
}
