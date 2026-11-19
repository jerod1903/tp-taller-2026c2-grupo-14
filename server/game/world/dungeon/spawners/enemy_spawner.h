#ifndef ENEMY_SPAWNER_H
#define ENEMY_SPAWNER_H

#include <random>

class DungeonLevel;

// Cómo aparece un tipo de enemigo en un nivel: cuántos, juntos o sueltos. Sin estado: una por
// tipo. Cada vez que se repuebla un nivel aparece un solo tipo.
class EnemySpawner {
public:
    // Pone sus enemigos en celdas libres del nivel, lejos de los portales. Si no hay lugar, pone
    // los que entren (o ninguno).
    virtual void spawn(DungeonLevel& level) const = 0;

    virtual ~EnemySpawner() = default;
};

// Uno de los tipos de enemigo, sorteado (todos con la misma probabilidad).
const EnemySpawner& random_spawner(std::mt19937& rng);

#endif  // ENEMY_SPAWNER_H
