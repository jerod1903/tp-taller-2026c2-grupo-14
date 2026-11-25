#ifndef FINISH_CHEAT_H
#define FINISH_CHEAT_H

#include <string>

#include "server/game/cheats/cheat.h"

// "$finish": les avisa a todos los conectados que se completó el juego, como al completar el centro
// comunitario. Provisorio, para probar la pantalla de felicitación: cuando esté el cheat de
// completar todos los sets, ese hace lo mismo y este se saca. Sin payload.
class FinishCheat: public Cheat {
public:
    void apply(Player& player, GameContext& context, const std::string& payload) const override;
};

#endif  // FINISH_CHEAT_H
