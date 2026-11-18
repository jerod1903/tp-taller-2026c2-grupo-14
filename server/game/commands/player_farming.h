#ifndef PLAYER_FARMING_H
#define PLAYER_FARMING_H

#include "server/game/characters/player.h"
#include "server/game/world/farm/milking.h"

// Lo que el jugador hace en la granja: lo que usan Interact y las herramientas.
class PlayerFarming {
public:
    // Cava la celda que tiene adelante, si está en la granja. true si cavó.
    virtual bool dig(const Player& player) = 0;
    // Planta en la celda que tiene adelante, si está en la granja y es tierra arada vacía.
    // true si plantó.
    virtual bool plant(const Player& player, CropType crop) = 0;
    // Riega el cultivo de la celda que tiene adelante, si está en la granja. true si regó.
    virtual bool water(const Player& player) = 0;
    // Cosecha el cultivo listo de la celda que tiene adelante, si está en la granja.
    virtual const Outcome& harvest(Player& player) = 0;
    // Ordeña la vaca lista más cercana que tenga al alcance.
    virtual const Outcome& milk(Player& player) = 0;

    virtual ~PlayerFarming() = default;
};

#endif  // PLAYER_FARMING_H
