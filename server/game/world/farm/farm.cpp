#include "server/game/world/farm/farm.h"

#include <memory>
#include <string>
#include <utility>
#include <vector>

Farm::Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin):
        Location(std::move(name), std::move(grid)), cabin_pos(cabin_pos), cabin(std::move(cabin)) {
    add_portal(Portal{cabin_pos, this->cabin.get(), this->cabin->get_door_pos()});
    this->cabin->add_portal(Portal{this->cabin->get_door_pos(), this, cabin_pos});
}

const Position& Farm::get_cabin_pos() const { return cabin_pos; }

Cabin& Farm::get_cabin() { return *cabin; }

const Cabin& Farm::get_cabin() const { return *cabin; }

SpawnPoint Farm::bed_spawn(uint32_t player_id) { return cabin->bed_spawn(player_id); }

void Farm::add_animal(AnimalKind kind, const Position& pos) {
    animals.push_back(std::make_unique<Animal>(next_animal_id++, kind, pos, *this));
    add_entity(*animals.back());
}

const std::vector<std::unique_ptr<Animal>>& Farm::get_animals() const { return animals; }

void Farm::update(float dt) {
    update_entities(dt);
    cabin->update(dt);
}
