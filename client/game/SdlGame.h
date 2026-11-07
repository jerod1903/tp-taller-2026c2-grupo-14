#pragma once

#include "client/app/IGameView.h"

class IGameProtocol;

// La partida en SDL: arma un Cliente nuevo por partida sobre la conexion de ClientApp.
class SdlGame: public IGameView {
public:
    explicit SdlGame(IGameProtocol& protocol);
    void run(const GameSession& session) override;

private:
    IGameProtocol& protocol;
};
