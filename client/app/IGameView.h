#pragma once

#include "client/app/GameSession.h"

// La partida, visto desde ClientApp.
class IGameView {
public:
    // Juega la partida hasta que el jugador sale.
    virtual void run(const GameSession& session) = 0;
    virtual ~IGameView() = default;
};
