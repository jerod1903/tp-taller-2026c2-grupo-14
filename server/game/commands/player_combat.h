#ifndef PLAYER_COMBAT_H
#define PLAYER_COMBAT_H

#include "server/game/characters/player.h"
#include "server/game/world/position.h"

// Lo que el jugador hace al pelear: lo que usa la espada.
class PlayerCombat {
public:
    // Le pega al enemigo que esté en `cell` (o el más cercano a ella, porque se mueven entre
    // celdas), solo si el jugador está en la mazmorra y la celda está pegada a la suya. true si
    // le pegó.
    virtual bool attack(Player& player, const Position& cell) = 0;

    virtual ~PlayerCombat() = default;
};

#endif  // PLAYER_COMBAT_H
