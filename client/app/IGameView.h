#pragma once

#include <optional>

#include "client/app/GameSession.h"
#include "common/game_dto.h"

// La partida, visto desde ClientApp.
class IGameView {
public:
    // Juega la partida hasta que termina. Devuelve el motivo que mando el servidor en GameEnded,
    // o nullopt si se corto la conexion.
    virtual std::optional<EndReason> run(const GameSession& session) = 0;
    virtual ~IGameView() = default;
};
