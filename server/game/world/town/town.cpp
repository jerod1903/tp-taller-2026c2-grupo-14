#include "server/game/world/town/town.h"

#include <memory>
#include <string>
#include <utility>

Town::Town(std::string name, Grid grid, const DoorCells& market_door,
           std::unique_ptr<Interior> market, const DoorCells& community_center_door,
           std::unique_ptr<Interior> community_center):
        Location(std::move(name), std::move(grid)),
        market(std::move(market)),
        community_center(std::move(community_center)) {
    this->market->link_to(*this, market_door);
    this->community_center->link_to(*this, community_center_door);
}

Interior& Town::get_market() { return *market; }

Interior& Town::get_community_center() { return *community_center; }

void Town::update(float dt) {
    update_entities(dt);
    market->update(dt);
    community_center->update(dt);
}
