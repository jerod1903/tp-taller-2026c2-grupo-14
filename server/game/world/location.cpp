#include "server/game/world/location.h"

#include <algorithm>
#include <cstddef>
#include <functional>
#include <optional>
#include <random>
#include <utility>
#include <vector>

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

std::optional<Position> Location::random_free_cell(
        std::mt19937& rng, const std::function<bool(const Position&)>& filter) const {
    // Se juntan todas las que sirven y se sortea una: así siempre encuentra lugar si lo hay
    std::vector<Position> candidates = free_cells(filter);
    if (candidates.empty()) {
        return std::nullopt;
    }
    std::uniform_int_distribution<std::size_t> pick(0, candidates.size() - 1);
    return candidates[pick(rng)];
}

std::vector<Position> Location::free_cells(
        const std::function<bool(const Position&)>& filter) const {
    std::vector<Position> cells;
    for (int y = 0; y < grid.get_height(); ++y) {
        for (int x = 0; x < grid.get_width(); ++x) {
            Position pos{x, y};
            if (is_free(pos) && (!filter || filter(pos))) {
                cells.push_back(pos);
            }
        }
    }
    return cells;
}

bool Location::is_free(const Position& pos) const {
    return is_walkable(pos) && !portal_at(pos).has_value() &&
           std::none_of(entities.begin(), entities.end(),
                        [&pos](const Entity* entity) { return entity->get_position() == pos; });
}

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
