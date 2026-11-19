#include "server/game/snapshot_builder.h"

#include <cmath>
#include <map>
#include <memory>
#include <string>

#include "server/game/world/dungeon/flame.h"

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
                       static_cast<uint32_t>(player.get_gold()),
                       player.is_unconscious()};
}

EntityState entity_of(uint32_t player_id, const Player& player) {
    Position delta = step(Position{0, 0}, player.get_direction());
    return EntityState{player_id,
                       EntityType::Player,
                       player.get_location().get_name(),
                       to_wire(player.get_x()),
                       to_wire(player.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y),
                       player.is_typing() ? Emote::Typing : Emote::None};
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
                       static_cast<int8_t>(delta.y),
                       animal.is_ready() ? Emote::CowMilkReady : Emote::None};
}

EntityState entity_of(const Enemy& enemy) {
    Position delta = step(Position{0, 0}, enemy.get_direction());
    return EntityState{enemy.get_id(),
                       enemy.entity_type(),
                       enemy.get_location().get_name(),
                       to_wire(enemy.get_x()),
                       to_wire(enemy.get_y()),
                       static_cast<int8_t>(delta.x),
                       static_cast<int8_t>(delta.y),
                       Emote::None,
                       enemy.action()};
}

EntityState entity_of(const Flame& flame) {
    return EntityState{flame.get_id(),
                       flame.entity_type(),
                       flame.get_location().get_name(),
                       to_wire(flame.get_x()),
                       to_wire(flame.get_y()),
                       0,
                       0,
                       Emote::None,
                       EntityAction::None};
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
    const GameClock& clock = world.get_clock();
    Snapshot snapshot{tick, clock.get_day(), static_cast<uint16_t>(clock.get_minute_of_day()), {},
                      {}};
    for (const auto& [player_id, player]: world.get_players()) {
        snapshot.players.push_back(state_of(player_id, *player));
        snapshot.entities.push_back(entity_of(player_id, *player));
    }
    for (const std::unique_ptr<Animal>& animal: world.get_animals()) {
        snapshot.entities.push_back(entity_of(*animal));
    }
    const Dungeon& dungeon = world.get_dungeon();
    for (int depth = 0; depth < Dungeon::LEVEL_COUNT; ++depth) {
        for (const std::unique_ptr<Enemy>& enemy: dungeon.level(depth).get_enemies()) {
            snapshot.entities.push_back(entity_of(*enemy));
        }
        for (const Flame* flame: dungeon.level(depth).get_flames()) {
            snapshot.entities.push_back(entity_of(*flame));
        }
    }
    const Cabin& cabin = world.get_cabin();
    for (const auto& [player_id, slot]: cabin.get_beds()) {
        snapshot.entities.push_back(bed_of(player_id, slot, cabin.get_name()));
    }
    return snapshot;
}

FarmState SnapshotBuilder::farm_state(const Farm& farm) {
    FarmState state{farm.get_name(), {}};
    for (const auto& [pos, plot]: farm.get_plots()) {
        state.tiles.push_back(FarmTile{static_cast<uint16_t>(pos.x), static_cast<uint16_t>(pos.y),
                                       plot.soil, plot.crop.get_type(), plot.crop.get_stage(),
                                       plot.crop.needs_water(), plot.crop.is_ready()});
    }
    for (const auto& [pos, item]: farm.get_ground_items()) {
        state.items.push_back(
                GroundItem{static_cast<uint16_t>(pos.x), static_cast<uint16_t>(pos.y), item});
    }
    return state;
}
