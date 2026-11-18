#ifndef LOCATION_H
#define LOCATION_H

#include <functional>
#include <map>
#include <optional>
#include <random>
#include <string>
#include <vector>

#include "common/farm_dto.h"
#include "server/game/world/grid.h"
#include "server/game/world/portal.h"
#include "server/game/world/position.h"

class Entity;
class Outcome;
class Player;

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

    // Deja un objeto en el piso de la celda. false si ya había uno (y entonces no lo deja).
    bool drop(const Position& pos, ItemType item);
    // Los objetos tirados en el piso, uno por celda.
    const std::map<Position, ItemType>& get_ground_items() const;
    // Le da al jugador, que está en esta Location, lo que haya en el piso de su celda o de la de
    // adelante. Si no le entra en el inventario, queda en el piso.
    const Outcome& pick_up(Player& player);
    // true si algo de la Location cambió desde la última vez que se preguntó (el piso y lo que
    // marque cada subclase), para mandárselo a los jugadores.
    bool take_changes();

    virtual void update(float dt) = 0;

    virtual ~Location() = default;

protected:
    std::string name;
    Grid grid;
    std::vector<Portal> portals;
    std::vector<Entity*> entities;
    std::map<Position, ItemType> ground_items;
    bool changed = false;

    // Llama al update() de cada entidad. Recorre una copia: una entidad puede darse de baja.
    void update_entities(float dt);

private:
    // Se puede pisar, no hay ninguna entidad encima y no es un portal.
    bool is_free(const Position& pos) const;
    // Todas las celdas libres que cumplen `filter` (sin filtro, todas las libres).
    std::vector<Position> free_cells(const std::function<bool(const Position&)>& filter) const;
};

#endif  // LOCATION_H
