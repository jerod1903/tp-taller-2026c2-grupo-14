#ifndef ENEMY_SPAWNER_H
#define ENEMY_SPAWNER_H

#include <random>

class DungeonLevel;

// Cómo aparece un tipo de enemigo en un nivel: cuántos, juntos o sueltos. Sin estado: una por
// tipo. Cada vez que se repuebla un nivel aparecen uno o más grupos, cada uno de un tipo sorteado.
class EnemySpawner {
public:
    // Pone sus enemigos en celdas libres del nivel, lejos de los portales. Si no hay lugar, pone
    // los que entren (o ninguno).
    virtual void spawn(DungeonLevel& level) const = 0;

    virtual ~EnemySpawner() = default;
};

// Uno de los tipos de enemigo, sorteado (todos con la misma probabilidad). Sin `with_skull`, la
// calavera no entra en el sorteo: hay una sola por nivel.
const EnemySpawner& random_spawner(std::mt19937& rng, bool with_skull = true);

#endif  // ENEMY_SPAWNER_H
