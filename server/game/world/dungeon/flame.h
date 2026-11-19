#ifndef FLAME_H
#define FLAME_H

#include <cstdint>
#include <vector>

#include "common/game_dto.h"
#include "server/game/characters/creature.h"
#include "server/game/characters/entity.h"
#include "server/game/world/point.h"

class DungeonLevel;

// Azules, las básicas; verdes, el doble de daño a la mitad de velocidad.
enum class FlameKind {
    Blue,
    Green,
};

// Una llama que avanza en línea recta, en cualquier ángulo. Quema al primero de `targets` que toca
// y se apaga; también se apaga contra una pared o después de recorrer `range` celdas. No sabe
// quién la tiró: la puede crear cualquiera (una calavera, una trampa), que es su dueño. Mientras
// existe está anotada en el nivel, para que el snapshot la encuentre. Sus valores salen de
// [enemies.flame] en server_config.toml.
class Flame: public Entity {
public:
    // Sale de `from` hacia `towards`. `targets` es una lista que se actualiza sola (la de los
    // jugadores del nivel) y vive más que la llama.
    Flame(uint32_t id, const Point& from, const Point& towards, DungeonLevel& level,
          const std::vector<Creature*>& targets, FlameKind kind);
    ~Flame() override;

    EntityType entity_type() const;
    // El centro, en celdas, para que el cliente la dibuje donde está.
    float get_x() const;
    float get_y() const;
    // Ya se apagó: su dueño la puede destruir.
    bool is_gone() const;

    void update(float dt) override;

private:
    DungeonLevel& level;
    const std::vector<Creature*>& targets;
    FlameKind kind;
    Point center;
    // Hacia dónde va, de largo 1
    Point direction;
    float speed;
    int damage;
    float range_left;
    bool gone = false;

    // El primero de `targets` que toca, o nullptr
    Creature* touched() const;
};

#endif  // FLAME_H
