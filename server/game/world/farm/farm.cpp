#include "server/game/world/farm/farm.h"

#include <utility>

Farm::Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin):
        Location(std::move(name), std::move(grid)), cabin_pos(cabin_pos), cabin(std::move(cabin)) {
    add_portal(Portal{cabin_pos, this->cabin.get(), this->cabin->get_door_pos()});
    this->cabin->add_portal(Portal{this->cabin->get_door_pos(), this, cabin_pos});
}

const Position& Farm::get_cabin_pos() const { return cabin_pos; }

Cabin& Farm::get_cabin() { return *cabin; }

void Farm::update(float dt) {
    update_entities(dt);
    cabin->update(dt);
}
