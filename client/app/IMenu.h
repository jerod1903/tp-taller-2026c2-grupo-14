#pragma once

#include <optional>

#include "client/app/GameSession.h"

// El menu previo a la partida, visto desde ClientApp.
class IMenu {
public:
    // Muestra el menu hasta que el jugador entra a una partida. nullopt si cerro el menu.
    virtual std::optional<GameSession> run() = 0;
    // Avisa que se corto la conexion con el servidor durante la partida.
    virtual void showConnectionLost() = 0;
    virtual ~IMenu() = default;
};
