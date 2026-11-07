#include "client/client.h"

#include "client/game/game_loop.h"
#include "client/gui/interfaz.h"
#include "client/network/enviador.h"
#include "client/network/recibidor.h"

Cliente::Cliente(IGameProtocol& protocolo): protocolo(protocolo) {}

std::optional<EndReason> Cliente::run(const GameSession& session) {
    Enviador enviador(protocolo);
    Recibidor recibidor(protocolo);
    Interfaz interfaz;
    GameLoop game_loop(enviador, recibidor, interfaz, session.playerId);
    return game_loop.ejecutar();
}
