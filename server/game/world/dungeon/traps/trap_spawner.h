#ifndef TRAP_SPAWNER_H
#define TRAP_SPAWNER_H

class DungeonLevel;

// Pone las trampas de un nivel al azar: `spikes` pinches en celdas libres y `flamethrowers`
// lanzallamas en celdas libres pegadas a una pared que tengan arriba, a la izquierda o a la
// derecha (apuntan para el otro lado; para arriba no hay dibujo). Lejos de los portales, como los
// enemigos. Si no hay lugar, pone las que entren.
class TrapSpawner {
public:
    void spawn(DungeonLevel& level) const;

private:
    void spawn_spikes(DungeonLevel& level, float start) const;
    void spawn_flamethrower(DungeonLevel& level, float start) const;
};

#endif  // TRAP_SPAWNER_H
