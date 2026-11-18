#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/creature.h"

// Un enemigo de la mazmorra. Por defecto pelea cuerpo a cuerpo: si ve a un jugador lo persigue y,
// cuando lo alcanza, le pega cada un tiempo. Los que pelean distinto (a distancia) redefinen
// react().
class Enemy: public Creature {
public:
    // `visitors`: los jugadores que están en su nivel. Es la lista del nivel, que se actualiza en
    // cada tick; el nivel es dueño del enemigo, así que la lista vive más que él.
    Enemy(uint32_t id, const Position& pos, Location& location, const ServerConfig::Fighter& stats,
          const std::vector<Creature*>& visitors);

    // Llama a on_death() la primera vez que la vida llega a 0.
    void take_damage(int amount) override;

    // Drops, revivir, etc. Por defecto no hace nada.
    virtual void on_death();

    // Si ya no vuelve: el nivel lo da de baja. Por defecto, en cuanto muere.
    virtual bool is_gone() const;

    // Cuenta el tiempo entre golpes, decide qué hacer con los visitantes (react) y camina. Muerto
    // no hace nada.
    void update(float dt) override;

protected:
    // Vida con la que aparece (y con la que revive)
    int max_health;

    // Persigue al visitante más cercano que vea y le pega si lo alcanza; sin nadie a la vista se
    // queda quieto.
    virtual void react();

    // El jugador vivo más cercano que ve, o nullptr si no ve a ninguno.
    Creature* nearest_visible() const;

private:
    float sight;
    float reach;
    float cooldown;
    float cooldown_left = 0;  // el primer golpe sale en cuanto llega al lado
    const std::vector<Creature*>& visitors;

    // Está dentro de su vista y no hay paredes en el medio.
    bool sees(const Creature& target) const;
    // Recorre las celdas de la recta entre los dos: ninguna puede ser una pared.
    bool clear_line_to(const Creature& target) const;
    float distance_to(const Creature& target) const;
};

#endif  // ENEMY_H
