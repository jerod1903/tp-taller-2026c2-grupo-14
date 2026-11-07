#ifndef CLIENT_H
#define CLIENT_H

#include <optional>

#include "client/app/IGameView.h"
#include "client/network/IGameProtocol.h"

// La partida en SDL. Cada run() arma lo de una partida (ventana, hilo que recibe, loop) y lo
// destruye al terminar; la conexion es la de ClientApp y sigue viva para el menu.
class Cliente: public IGameView {
public:
    // protocolo: la conexion ya dentro de la partida (despues de entrar desde el lobby)
    explicit Cliente(IGameProtocol& protocolo);
    std::optional<EndReason> run(const GameSession& session) override;

private:
    IGameProtocol& protocolo;
};

#endif
