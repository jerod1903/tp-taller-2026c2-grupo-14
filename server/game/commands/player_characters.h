#ifndef PLAYER_CHARACTERS_H
#define PLAYER_CHARACTERS_H

#include <cstdint>

#include "server/game/characters/player.h"

// Los personajes de la partida: lo que usan Chat, Interact, SelectSlot, Typing y UseTool.
class PlayerCharacters {
public:
    // nullptr si el jugador no está en la partida.
    virtual Player* find_player(uint32_t player_id) = 0;

    virtual ~PlayerCharacters() = default;
};

#endif  // PLAYER_CHARACTERS_H
