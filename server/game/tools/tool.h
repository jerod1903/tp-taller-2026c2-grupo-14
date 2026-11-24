#ifndef TOOL_H
#define TOOL_H

#include "common/game_dto.h"
#include "server/game/world/position.h"

class Player;
class GameContext;

// Lo que pasa cuando el jugador usa el ítem que tiene elegido. Sin estado: una por tipo de ítem.
class Tool {
public:
    // Sobre la celda clickeada. true si hizo algo, y entonces cansa energy_cost().
    virtual bool use(Player& player, GameContext& context, const Position& cell) const = 0;
    virtual int energy_cost() const = 0;

    virtual ~Tool() = default;
};

// La herramienta de ese ítem, o nullptr si no se usa.
const Tool* tool_for(ItemType item);

// Pico, regadera, pala y espada: el jugador las lleva siempre, no se guardan en cofres.
bool is_work_tool(ItemType item);

#endif  // TOOL_H
