#ifndef SKULL_H
#define SKULL_H

#include <cstdint>
#include <memory>
#include <vector>

#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/dungeon/flame.h"

class DungeonLevel;

// Pelea a distancia y no se la puede matar: el jugador tiene que huir. Si ve a un jugador se queda
// donde está y le tira una llama cada cooldown; si no, deambula. Tira siempre llamas de su color
// (`kind`). Es dueña de sus llamas. Sus valores salen de [enemies.skull] en server_config.toml.
class Skull: public Enemy {
public:
    // Recibe el nivel (y no cualquier Location) porque de ahí saca los ids de sus llamas.
    Skull(uint32_t id, const Position& pos, DungeonLevel& level,
          const std::vector<Creature*>& visitors, FlameKind kind, uint32_t seed);

    EntityType entity_type() const override;
    // No le hace nada: ni vida ni dolor.
    void take_damage(int amount) override;
    // Lo de cualquier enemigo, y después mueve sus llamas y destruye las apagadas.
    void update(float dt) override;

    const std::vector<std::unique_ptr<Flame>>& get_flames() const;

protected:
    // Si ve a alguien, quieta, le tira una llama cuando puede; si no, deambula.
    void react(float dt) override;

private:
    DungeonLevel& level;
    FlameKind kind;
    std::vector<std::unique_ptr<Flame>> flames;

    void shoot(const Creature& target);
};

#endif  // SKULL_H
