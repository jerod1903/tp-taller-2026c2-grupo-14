#include "client/game/SdlGame.h"

#include "client/client.h"

SdlGame::SdlGame(IGameProtocol& protocol): protocol(protocol) {}

void SdlGame::run(const GameSession& session) {
    Cliente cliente(protocol, session.playerId);
    cliente.ejecutar();
}
