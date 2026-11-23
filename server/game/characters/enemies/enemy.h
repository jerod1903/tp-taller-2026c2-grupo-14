#ifndef ENEMY_H
#define ENEMY_H

#include <cstdint>
#include <random>
#include <vector>

#include "common/game_dto.h"
#include "server/config/server_config.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/wanderer.h"

// Lo que queda el cuerpo de un enemigo que no revive antes de que el nivel lo dé de baja, en
// segundos: así el cliente llega a mostrar cómo muere.
inline constexpr float CORPSE_TIME = 1.5f;

// Un enemigo de la mazmorra. Por defecto pelea cuerpo a cuerpo: si ve a un jugador lo persigue y,
// cuando lo alcanza, le pega cada un tiempo; si no ve a nadie, deambula. Los que pegan distinto
// (la calavera tira una llama) redefinen strike().
class Enemy: public Creature {
public:
    // `visitors`: los jugadores que están en su nivel. Es la lista del nivel, que se actualiza en
    // cada tick; el nivel es dueño del enemigo, así que la lista vive más que él.
    // `attack_time` y `hurt_time`: cuánto tarda en pegar y cuánto queda dolorido por un golpe.
    // `seed`: para sus sorteos (hacia dónde deambula).
    Enemy(uint32_t id, const Position& pos, Location& location, const ServerConfig::Fighter& stats,
          const std::vector<Creature*>& visitors, float attack_time, float hurt_time,
          uint32_t seed);

    // Queda dolorido un rato. Llama a on_death() cada vez que la vida llega a 0.
    void take_damage(int amount) override;

    // Lo que está haciendo: muerto, dolorido, pegando o nada (camina o está quieto).
    EntityAction action() const;

    // Qué enemigo es, para que el cliente sepa cómo dibujarlo.
    virtual EntityType entity_type() const = 0;

    // Drops, revivir, etc. Por defecto no hace nada.
    virtual void on_death();

    // Un golpe de un jugador: le saca `damage` de vida y devuelve las monedas que suelta con ese
    // golpe, que son para quien lo dio. Solo suelta si el golpe lo mata y es la primera vez que
    // muere (el que revive no vuelve a soltar, así no se lo puede farmear); si no, 0.
    int take_hit(int damage);

    // Si ya no vuelve: el nivel lo da de baja. Por defecto, pasados CORPSE_TIME segundos de que
    // muere.
    virtual bool is_gone() const;

    // Vivo, cuenta el tiempo entre golpes, decide qué hacer con los visitantes (react) y camina.
    // Muerto, hace lo suyo (while_dead).
    void update(float dt) override;

protected:
    // Vida con la que aparece (y con la que revive)
    int max_health;
    std::mt19937 rng;

    // Persigue al visitante más cercano que vea y le pega si lo alcanza; sin nadie a la vista
    // deambula.
    virtual void react(float dt);

    // El golpe en sí. Por defecto, cuerpo a cuerpo: le saca su daño a `target`.
    virtual void strike(Creature& target);

    // Lo que hace mientras está muerto. Por defecto cuenta el tiempo que le queda al cuerpo; los
    // que reviven cuentan para revivir.
    virtual void while_dead(float dt);

    // El jugador vivo más cercano que ve, o nullptr si no ve a ninguno.
    Creature* nearest_visible() const;
    // Los jugadores que están en su nivel
    const std::vector<Creature*>& get_visitors() const;
    float distance_to(const Creature& target) const;

private:
    float sight;
    float reach;
    float cooldown;
    float cooldown_left = 0;  // el primer golpe sale en cuanto llega al lado
    float attack_time;
    float hurt_time;
    float attacking_left = 0;
    float hurt_left = 0;
    float corpse_left = 0;
    // Ya soltó su oro: el que revive no lo vuelve a soltar
    bool looted = false;
    const std::vector<Creature*>& visitors;
    Wanderer wanderer;

    // Las monedas que puede soltar (su sección de server_config.toml), o nullptr si no suelta nada
    // (la calavera, que no se puede matar). Privada: la redefine cada enemigo, pero solo la usa
    // take_hit.
    virtual const ServerConfig::Loot* loot_table() const;
    // Con su loot_table(): si suelta y cuánto
    int roll_gold();
    // Si ya pasó el cooldown, pega (strike) y vuelve a contar el cooldown y el tiempo que queda
    // pegando.
    void try_attack(Creature& target);
    // Está dentro de su vista y no hay paredes en el medio.
    bool sees(const Creature& target) const;
    // Recorre las celdas de la recta entre los dos: ninguna puede ser una pared.
    bool clear_line_to(const Creature& target) const;
};

#endif  // ENEMY_H
