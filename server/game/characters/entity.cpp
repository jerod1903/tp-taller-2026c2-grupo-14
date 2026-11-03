#include "server/game/characters/entity.h"

#include "server/game/world/location.h"

Entity::Entity(uint32_t id, const Position& pos, Location& location):
        id(id), pos(pos), location(&location) {}

uint32_t Entity::get_id() const { return id; }

const Position& Entity::get_position() const { return pos; }

Location& Entity::get_location() const { return *location; }

void Entity::relocate(Location& target, const Position& arrival) {
    location->remove_entity(*this);
    location = &target;
    pos = arrival;
    target.add_entity(*this);
}
