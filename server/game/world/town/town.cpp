#include "server/game/world/town/town.h"

#include <memory>
#include <string>
#include <utility>

Town::Town(std::string name, Grid grid, const Position& market_pos,
           std::unique_ptr<Interior> market):
        Location(std::move(name), std::move(grid)), market(std::move(market)) {
    this->market->link_to(*this, market_pos);
}

Interior& Town::get_market() { return *market; }

void Town::update(float dt) {
    update_entities(dt);
    market->update(dt);
}
