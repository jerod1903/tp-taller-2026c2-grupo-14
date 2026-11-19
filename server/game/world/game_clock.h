#ifndef GAME_CLOCK_H
#define GAME_CLOCK_H

#include <cstdint>

// La hora de la partida. Un día de juego va de 6:00 a 2:00 (1200 minutos de juego) y dura
// day_seconds segundos reales.
class GameClock {
public:
    explicit GameClock(float day_seconds);

    uint16_t get_day() const;
    float get_elapsed() const;        // segundos transcurridos en el día
    float get_minute_of_day() const;  // minutos de juego desde las 6:00
    float get_day_seconds() const;    // segundos que dura un día
    // true si con este dt terminó el día. Terminado, se queda en las 2:00 (y sigue devolviendo
    // true) hasta next_day().
    bool update(float dt);
    // Termina el día antes de las 2:00 (cayeron todos inconscientes): pasa a las 2:00.
    void end_day();
    // Arranca el día siguiente a las 6:00.
    void next_day();
    // Arranca el día `day` a las 6:00 (al cargar una partida guardada).
    void start_day(uint16_t day);

private:
    float day_seconds;
    uint16_t day = 1;
    float elapsed = 0.0f;
};

#endif  // GAME_CLOCK_H
