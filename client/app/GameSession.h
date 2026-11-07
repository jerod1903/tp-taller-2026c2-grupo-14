#pragma once

#include <cstdint>

// La partida a la que entro el jugador: lo que el menu le pasa al juego.
struct GameSession {
    uint32_t gameId;
    uint32_t playerId;  // identifica al jugador propio en los snapshots
};
