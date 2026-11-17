#ifndef LOCATION_H
#define LOCATION_H

#include <functional>
#include <optional>
#include <random>
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

    virtual bool is_walkable(const Position& pos) const;

    // std::nullopt si no hay portal en esa posición.
    std::optional<Portal> portal_at(const Position& pos) const;

    void add_portal(const Portal& portal);

    // Una celda libre al azar donde poner algo (un enemigo, un mineral), o std::nullopt si no hay
    // ninguna. Libre: se puede pisar, no hay ninguna entidad encima y no es un portal. `filter`
    // descarta las que no le sirven a quien pide (por ejemplo, las cercanas a la entrada); sin
    // filtro sirve cualquiera. Todas las que cumplen tienen la misma chance.
    std::optional<Position> random_free_cell(
            std::mt19937& rng, const std::function<bool(const Position&)>& filter = nullptr) const;

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

private:
    // Se puede pisar, no hay ninguna entidad encima y no es un portal.
    bool is_free(const Position& pos) const;
    // Todas las celdas libres que cumplen `filter` (sin filtro, todas las libres).
    std::vector<Position> free_cells(const std::function<bool(const Position&)>& filter) const;
};

#endif  // LOCATION_H
