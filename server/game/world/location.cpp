#include "server/game/world/location.h"

#include <algorithm>
#include <utility>

#include "server/game/characters/entity.h"

Location::Location(std::string name, Grid grid): name(std::move(name)), grid(std::move(grid)) {}

const std::string& Location::get_name() const { return name; }

const Grid& Location::get_grid() const { return grid; }

bool Location::is_walkable(const Position& pos) const {
    return grid.in_bounds(pos) && grid.at(pos).is_walkable();
}

std::optional<Portal> Location::portal_at(const Position& pos) const {
    auto it = std::find_if(portals.begin(), portals.end(),
                           [&pos](const Portal& portal) { return portal.origin == pos; });
    if (it == portals.end()) {
        return std::nullopt;
    }
    return *it;
}

void Location::add_portal(const Portal& portal) { portals.push_back(portal); }

void Location::add_entity(Entity& entity) { entities.push_back(&entity); }

void Location::remove_entity(const Entity& entity) {
    entities.erase(std::remove(entities.begin(), entities.end(), &entity), entities.end());
}

const std::vector<Entity*>& Location::get_entities() const { return entities; }

void Location::update_entities(float dt) {
    std::vector<Entity*> current = entities;
    for (Entity* entity: current) {
        entity->update(dt);
    }
}
