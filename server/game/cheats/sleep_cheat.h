#ifndef SLEEP_CHEAT_H
#define SLEEP_CHEAT_H

#include <string>

#include "server/game/cheats/cheat.h"

// "$sleep": termina el día en el acto, como si fueran las 2:00 (para probar el paso de día sin
// esperar). Sin payload.
class SleepCheat: public Cheat {
public:
    void apply(Player& player, GameContext& context, const std::string& payload) const override;
};

#endif  // SLEEP_CHEAT_H
