#ifndef VAMPIRE_H
#define VAMPIRE_H

#include <cstdint>
#include <optional>
#include <vector>

#include "server/game/characters/enemies/enemy.h"

enum class VampireKind { Vampire, Vampiress };

// Pelea cuerpo a cuerpo y aparece en pareja: la vampira es más rápida y el vampiro pega más
// fuerte. Si muere uno, el otro entra en frenesí y duplica lo suyo. Muerto, revive si cerca de su
// cuerpo un jugador muere o queda gravemente herido. Sus valores salen de [enemies.vampire] en
// server_config.toml.
class Vampire: public Enemy {
public:
    Vampire(uint32_t id, const Position& pos, Location& location,
            const std::vector<Creature*>& visitors, VampireKind kind, uint32_t seed);

    // Los vuelve pareja. Los dos tienen que ser del mismo nivel, que es dueño de los dos. Todo
    // vampiro tiene que estar emparejado antes del primer update().
    void pair_with(Vampire& other);
    bool is_in_frenzy() const;
    // Avisos de su pareja: si murió, entra en frenesí (si está vivo); si revivió, se le pasa.
    void partner_died();
    void partner_revived();

    EntityType entity_type() const override;
    // Le avisa a su pareja.
    void on_death() override;
    // Nunca se va: puede revivir. Se va cuando el nivel se repuebla.
    bool is_gone() const override;
    // La vampira en frenesí, al doble.
    float speed() const override;

protected:
    // El vampiro en frenesí, al doble.
    int attack_damage() const override;
    // Espera la sangre y después el tiempo para revivir.
    void while_dead(float dt) override;

private:
    VampireKind kind;
    Vampire* partner = nullptr;
    bool frenzy = false;
    std::optional<float> revive_left;  // desde que cayó sangre cerca

    // Algún jugador a menos de blood_reach de su cuerpo murió o está gravemente herido.
    bool blood_nearby() const;
    // Con la vida llena; le avisa a su pareja. Si la pareja sigue muerta, entra en frenesí.
    void revive();
};

#endif  // VAMPIRE_H
