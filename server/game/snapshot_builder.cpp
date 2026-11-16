#include "server/game/snapshot_builder.h"

#include <cmath>
#include <map>
#include <memory>
#include <string>

namespace {
uint32_t to_wire(float cells) { return static_cast<uint32_t>(std::lround(cells * POSITION_SCALE)); }

PlayerState state_of(uint32_t player_id, const Player& player) {
    Position delta = step(Position{0, 0}, player.get_direction());
    return PlayerState{player_id,
                       player.get_location().get_name(),
                       to_wire(player.get_x()),
                       to_wire(player.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y),
                       static_cast<uint16_t>(player.get_health()),
                       static_cast<uint16_t>(player.get_energy()),
                       static_cast<uint32_t>(player.get_gold())};
}

EntityState entity_of(uint32_t player_id, const Player& player) {
    Position delta = step(Position{0, 0}, player.get_direction());
    return EntityState{player_id,
                       EntityType::Player,
                       player.get_location().get_name(),
                       to_wire(player.get_x()),
                       to_wire(player.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y)};
}

const std::map<AnimalKind, EntityType> ENTITY_TYPE_OF = {
        {AnimalKind::Cow, EntityType::Cow},
        {AnimalKind::Chicken, EntityType::Chicken},
};

EntityState entity_of(const Animal& animal) {
    Position delta = step(Position{0, 0}, animal.get_direction());
    return EntityState{animal.get_id(),
                       ENTITY_TYPE_OF.at(animal.get_kind()),
                       animal.get_location().get_name(),
                       to_wire(animal.get_x()),
                       to_wire(animal.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y)};
}

// La celda de arriba a la izquierda de la cama, en milésimas de celda
EntityState bed_of(uint32_t player_id, const Position& slot, const std::string& area_id) {
    return EntityState{player_id,
                       EntityType::Bed,
                       area_id,
                       static_cast<uint32_t>(slot.x) * POSITION_SCALE,
                       static_cast<uint32_t>(slot.y) * POSITION_SCALE,
                       0,
                       0};
}
}  // namespace

Snapshot SnapshotBuilder::build(uint32_t tick, const World& world) {
    Snapshot snapshot{tick, {}, {}};
    for (const auto& [player_id, player]: world.get_players()) {
        snapshot.players.push_back(state_of(player_id, *player));
        snapshot.entities.push_back(entity_of(player_id, *player));
    }
    for (const std::unique_ptr<Animal>& animal: world.get_animals()) {
        snapshot.entities.push_back(entity_of(*animal));
    }
    const Cabin& cabin = world.get_cabin();
    for (const auto& [player_id, slot]: cabin.get_beds()) {
        snapshot.entities.push_back(bed_of(player_id, slot, cabin.get_name()));
    }
    return snapshot;
}
