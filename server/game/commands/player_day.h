#ifndef PLAYER_DAY_H
#define PLAYER_DAY_H

#include <cstdint>

// El fin del día: cada jugador da el Ok para pasar al siguiente. Lo usan DayReadyCommand y el
// cheat $sleep.
class PlayerDay {
public:
    // Si no se está votando, o ya lo había dado, no hace nada.
    virtual void ready_for_next_day(uint32_t player_id) = 0;
    // Termina el día ya, como si fueran las 2:00 (cheat). Si ya terminó, no hace nada.
    virtual void end_day() = 0;

    virtual ~PlayerDay() = default;
};

#endif  // PLAYER_DAY_H
