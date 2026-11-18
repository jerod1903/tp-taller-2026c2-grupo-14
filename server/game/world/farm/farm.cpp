#include "server/game/world/farm/farm.h"

#include <algorithm>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

Farm::Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin):
        Location(std::move(name), std::move(grid)), cabin_pos(cabin_pos), cabin(std::move(cabin)) {
    this->cabin->link_to(*this, cabin_pos);
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

int Farm::count_animals(AnimalKind kind) const {
    return static_cast<int>(std::count_if(
            animals.begin(), animals.end(),
            [kind](const std::unique_ptr<Animal>& animal) { return animal->get_kind() == kind; }));
}

void Farm::remove_animals(AnimalKind kind, int quantity) {
    if (count_animals(kind) < quantity) {
        throw std::invalid_argument("not enough animals in the farm");
    }
    for (auto it = animals.begin(); it != animals.end() && quantity > 0;) {
        if ((*it)->get_kind() != kind) {
            ++it;
            continue;
        }
        remove_entity(**it);
        it = animals.erase(it);
        --quantity;
    }
}

void Farm::update(float dt) {
    update_entities(dt);
    cabin->update(dt);
}
