#ifndef PLAYER_CHARACTERS_H
#define PLAYER_CHARACTERS_H

#include <cstdint>

#include "server/game/characters/player.h"
#include "server/game/world/position.h"

class Location;

// Los personajes de la partida: lo que usan Chat, Interact, SelectSlot, Typing, Click y los
// regalos.
class PlayerCharacters {
public:
    // nullptr si el jugador no está en la partida.
    virtual Player* find_player(uint32_t player_id) = 0;
    // El jugador parado en esa celda de `location`, o en la de abajo (es de dos casillas de alto);
    // nullptr si no hay ninguno.
    virtual Player* player_at(const Location& location, const Position& cell) = 0;

    virtual ~PlayerCharacters() = default;
};

#endif  // PLAYER_CHARACTERS_H
