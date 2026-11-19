#ifndef TRAP_H
#define TRAP_H

#include <cstdint>
#include <set>
#include <vector>

#include "common/game_dto.h"
#include "server/config/server_config.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/entity.h"

// Una trampa de la mazmorra, fija en su celda. Se activa por tiempo: active_time segundos activa y
// después idle_time inactiva, una y otra vez. Mientras está activa le saca su daño a cada uno de
// `targets` que esté en su zona, una sola vez por activación. Cada subclase dice cuál es su zona.
class Trap: public Entity {
public:
    // `targets`: una lista que se actualiza sola (la de los jugadores del nivel) y vive más que la
    // trampa. `start`: en qué momento del ciclo arranca, en segundos, para que no se activen
    // todas juntas.
    Trap(uint32_t id, const Position& pos, Location& location,
         const std::vector<Creature*>& targets, const ServerConfig::Trap& stats, float start);

    // Qué trampa es, para que el cliente sepa cómo dibujarla.
    virtual EntityType entity_type() const = 0;
    bool is_active() const;

    // Avanza el ciclo y, si está activa, lastima a los que estén en su zona.
    void update(float dt) override;

protected:
    // Si `cell` está en la zona que lastima
    virtual bool covers(const Position& cell) const = 0;

private:
    const std::vector<Creature*>& targets;
    int damage;
    float active_time;
    float cycle;  // active_time + idle_time
    float time;   // en qué momento del ciclo está: activa en [0, active_time)
    // Los que ya lastimó en esta activación
    std::set<const Creature*> hurt;

    void hurt_the_ones_inside();
};

#endif  // TRAP_H
