#include "server/game/world/farm/farm.h"

#include <algorithm>
#include <map>
#include <memory>
#include <optional>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/animals/chicken/chicken.h"
#include "server/game/characters/animals/cow/cow.h"
#include "server/game/characters/player.h"
#include "server/game/interactions/outcome.h"

Farm::Farm(std::string name, Grid grid, const Position& cabin_pos, std::unique_ptr<Cabin> cabin,
           const std::set<Position>& tilled):
        Location(std::move(name), std::move(grid)), cabin_pos(cabin_pos), cabin(std::move(cabin)) {
    this->cabin->link_to(*this, cabin_pos);
    for (const Position& pos: tilled) {
        plots.emplace(pos, Plot{SoilType::Tilled, Crop{}});
    }
}

const Position& Farm::get_cabin_pos() const { return cabin_pos; }

Cabin& Farm::get_cabin() { return *cabin; }

const Cabin& Farm::get_cabin() const { return *cabin; }

SpawnPoint Farm::bed_spawn(uint32_t player_id) { return cabin->bed_spawn(player_id); }

bool Farm::add_animal(AnimalKind kind, const Position& pos) {
    const ServerConfig::Animals& config = server_config().animals;
    switch (kind) {
        case AnimalKind::Cow:
            if (count_of(kind) >= config.cow.max_per_farm) {
                return false;
            }
            animals.push_back(std::make_unique<Cow>(next_animal_id++, pos, *this));
            break;
        case AnimalKind::Chicken:
            if (count_of(kind) >= config.chicken.max_per_farm) {
                return false;
            }
            animals.push_back(std::make_unique<Chicken>(next_animal_id++, pos, *this));
            break;
    }
    add_entity(*animals.back());
    return true;
}

const std::vector<std::unique_ptr<Animal>>& Farm::get_animals() const { return animals; }

void Farm::start_new_day() {
    for (const std::unique_ptr<Animal>& animal: animals) {
        animal->start_new_day();
    }
    collect_drops();
    minute = 0.0f;
    for (auto& [pos, plot]: plots) {
        if (plot.crop.is_planted()) {
            plot.crop.grow();
            changed = true;
        }
    }
}

void Farm::collect_drops() {
    for (const std::unique_ptr<Animal>& animal: animals) {
        if (std::optional<ItemType> dropped = animal->take_drop()) {
            // Si la celda ya tiene algo, el nuevo se pierde
            drop(animal->get_position(), *dropped);
        }
    }
}

int Farm::count_of(AnimalKind kind) const {
    return static_cast<int>(
            std::count_if(animals.begin(), animals.end(), [kind](const auto& animal) {
                return animal->get_kind() == kind;
            }));
}

bool Farm::dig(const Position& pos) {
    auto plot = plots.find(pos);
    if (plot != plots.end()) {
        if (plot->second.soil != SoilType::Dirt) {
            return false;
        }
        plot->second.soil = SoilType::Tilled;
        changed = true;
        return true;
    }
    if (!grid.in_bounds(pos) || !is_walkable(pos)) {
        return false;
    }
    switch (grid.at(pos).get_type()) {
        case TileType::Grass:
            plots.emplace(pos, Plot{SoilType::Dirt, Crop{}});
            break;
        case TileType::Dirt:
            plots.emplace(pos, Plot{SoilType::Tilled, Crop{}});
            break;
        default:
            return false;
    }
    changed = true;
    return true;
}

bool Farm::plant(const Position& pos, CropType type) {
    auto plot = plots.find(pos);
    if (type == CropType::None || plot == plots.end() || plot->second.soil != SoilType::Tilled ||
        plot->second.crop.is_planted()) {
        return false;
    }
    plot->second.crop = Crop{type, minute};
    changed = true;
    return true;
}

const Outcome& Farm::harvest(const Position& pos, Player& player) {
    auto plot = plots.find(pos);
    if (plot == plots.end() || !plot->second.crop.is_ready()) {
        return Outcome::not_here();
    }
    Crop& crop = plot->second.crop;
    const ServerConfig::Quality& config = server_config().crops.quality;
    std::uniform_real_distribution<float> luck(config.luck_min, config.luck_max);
    if (player.receive(crop.product(), 1, crop.quality(minute, luck(rng))) > 0) {
        return Outcome::inventory_full();
    }
    crop.harvested();
    changed = true;
    return Outcome::done();
}

bool Farm::water(const Position& pos) {
    auto plot = plots.find(pos);
    if (plot == plots.end() || !plot->second.crop.water(minute)) {
        return false;
    }
    changed = true;
    return true;
}

void Farm::update_time(float minute_of_day) {
    minute = minute_of_day;
    for (auto& [pos, plot]: plots) {
        changed |= plot.crop.update_time(minute);
    }
}

const std::map<Position, Plot>& Farm::get_plots() const { return plots; }


void Farm::update(float dt) {
    update_entities(dt);
    collect_drops();
    cabin->update(dt);
}
