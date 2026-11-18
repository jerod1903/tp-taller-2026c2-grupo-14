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

const std::map<Position, ItemType>& Farm::get_ground_items() const { return ground_items; }

std::optional<ItemType> Farm::pick_up(const Position& pos) {
    auto item = ground_items.find(pos);
    if (item == ground_items.end()) {
        return std::nullopt;
    }
    ItemType picked = item->second;
    ground_items.erase(item);
    return picked;
}

void Farm::start_new_day() {
    for (const std::unique_ptr<Animal>& animal: animals) {
        animal->start_new_day();
    }
    collect_drops();
}

void Farm::collect_drops() {
    for (const std::unique_ptr<Animal>& animal: animals) {
        if (std::optional<ItemType> dropped = animal->take_drop()) {
            // Si la celda ya tiene algo, el nuevo se pierde
            ground_items.emplace(animal->get_position(), *dropped);
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
    plot->second.crop = Crop{type};
    changed = true;
    return true;
}

std::optional<CropHarvest> Farm::harvest(const Position& pos) {
    auto plot = plots.find(pos);
    if (plot == plots.end() || !plot->second.crop.is_ready()) {
        return std::nullopt;
    }
    // La calidad todavía no se calcula.
    CropHarvest harvested{plot->second.crop.get_type(), ItemQuality::Base};
    plot->second.crop = Crop{};
    changed = true;
    return harvested;
}

const std::map<Position, Plot>& Farm::get_plots() const { return plots; }

bool Farm::take_changes() { return std::exchange(changed, false); }

void Farm::update(float dt) {
    update_entities(dt);
    collect_drops();
    cabin->update(dt);
}
