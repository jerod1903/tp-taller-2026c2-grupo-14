#ifndef PLAYER_CHARACTERS_H
#define PLAYER_CHARACTERS_H

#include <cstdint>
#include <memory>
#include <vector>

#include "server/game/characters/animals/animal.h"
#include "server/game/characters/player.h"

// Los personajes de la partida: lo que usan Chat, Interact, SelectSlot y Typing.
class PlayerCharacters {
public:
    // nullptr si el jugador no está en la partida.
    virtual Player* find_player(uint32_t player_id) = 0;
    // Los animales de la granja (para ordeñar).
    virtual const std::vector<std::unique_ptr<Animal>>& farm_animals() = 0;

    virtual ~PlayerCharacters() = default;
};

#endif  // PLAYER_CHARACTERS_H
