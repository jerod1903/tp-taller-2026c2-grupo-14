#ifndef LOCATION_H
#define LOCATION_H

#include <optional>
#include <string>
#include <vector>

#include "server/game/world/grid.h"
#include "server/game/world/portal.h"
#include "server/game/world/position.h"

class Entity;

// Un mapa del mundo (granja, pueblo, nivel de mazmorra). Es dueña de su grilla y sus portales.
class Location {
public:
    Location(std::string name, Grid grid);

    Location(const Location&) = delete;
    Location& operator=(const Location&) = delete;
    Location(Location&&) = delete;
    Location& operator=(Location&&) = delete;

    const std::string& get_name() const;
    const Grid& get_grid() const;

    bool is_walkable(const Position& pos) const;

    // std::nullopt si no hay portal en esa posición.
    std::optional<Portal> portal_at(const Position& pos) const;

    void add_portal(const Portal& portal);

    void add_entity(Entity& entity);
    void remove_entity(const Entity& entity);
    const std::vector<Entity*>& get_entities() const;

    virtual void update(float dt) = 0;

    virtual ~Location() = default;

protected:
    std::string name;
    Grid grid;
    std::vector<Portal> portals;
    std::vector<Entity*> entities;

    // Llama al update() de cada entidad. Recorre una copia: una entidad puede darse de baja.
    void update_entities(float dt);
};

#endif  // LOCATION_H
