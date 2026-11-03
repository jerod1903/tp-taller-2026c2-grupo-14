#ifndef ENTITY_H
#define ENTITY_H

#include <cstdint>

#include "server/game/world/position.h"

class Location;

// Cualquier cosa con posición dentro de una Location. `location` no es dueño. El constructor no la
// da de alta en la Location: lo hace quien es dueño de la entidad (el nivel, el pueblo, el World).
class Entity {
public:
    Entity(uint32_t id, const Position& pos, Location& location);

    // La Location guarda punteros a sus entidades: no se copia ni se mueve.
    Entity(const Entity&) = delete;
    Entity& operator=(const Entity&) = delete;
    Entity(Entity&&) = delete;
    Entity& operator=(Entity&&) = delete;

    uint32_t get_id() const;
    const Position& get_position() const;
    Location& get_location() const;

    // Se da de baja en la Location actual y de alta en `target`, en `arrival`.
    void relocate(Location& target, const Position& arrival);

    virtual void update(float dt) = 0;

    virtual ~Entity() = default;

protected:
    uint32_t id;
    Position pos;
    Location* location;
};

#endif  // ENTITY_H
