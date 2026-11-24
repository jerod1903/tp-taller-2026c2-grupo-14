#include "server/game/characters/player.h"

#include <algorithm>
#include <string>
#include <utility>
#include <vector>

#include "server/config/server_config.h"
#include "server/consts.h"
#include "server/game/characters/enemies/enemy.h"
#include "server/game/world/location.h"

namespace {

const std::vector<InventorySlot> STARTING_ITEMS = {{ItemType::Pickaxe, 1},
                                                   {ItemType::WateringCan, 1},
                                                   {ItemType::Shovel, 1},
                                                   {ItemType::Sword, 1}};

}  // namespace

Player::Player(uint32_t id, const Position& pos, Location& location, std::string name, int health,
               float base_speed, int gold, int energy):
        Creature(id, pos, location, health, base_speed, server_config().player.sword_damage),
        name(std::move(name)),
        gold(gold),
        energy(energy),
        max_health(health),
        max_energy(energy),
        inventory(ServerConstants::PLAYER_INVENTORY_SIZE, STARTING_ITEMS) {}

const std::string& Player::get_name() const { return name; }

int Player::get_gold() const { return gold; }

int Player::get_energy() const { return energy; }

void Player::add_gold(int amount) { gold += amount; }

void Player::spend_energy(int amount) { energy = std::max(0, energy - amount); }

bool Player::is_unconscious() const { return energy == 0 || !is_alive(); }

void Player::wake_up(const SpawnPoint& bed, bool slept_in_bed) {
    rest(slept_in_bed);
    lie_in(bed);
}

void Player::rest(bool slept_in_bed) {
    float recovery = slept_in_bed ? 1.0f : server_config().player.bad_night_recovery;
    health = std::max(1, static_cast<int>(max_health * recovery));
    energy = std::max(1, static_cast<int>(max_energy * recovery));
}

void Player::lie_in(const SpawnPoint& bed) {
    set_direction(Direction::NONE);
    relocate(bed.location, cell_of(bed.point));
    place_at(bed.point);
}

float Player::speed() const {
    bool tired = energy <= max_energy * server_config().player.tired_energy;
    return tired ? base_speed / 2 : base_speed;
}

void Player::remove_selected(uint8_t quantity) { inventory.remove_selected(quantity); }

ItemType Player::selected_item() const { return inventory.selected_item(); }

ItemType Player::item_at(uint8_t slot) const { return inventory.item_at(slot); }

std::optional<int> Player::strike(Enemy& target) {
    if (attack_cooldown_left > 0) {
        return std::nullopt;
    }
    attack_cooldown_left = server_config().player.attack_cooldown;
    attacking_left = server_config().player.attack_time;
    return target.take_hit(attack_damage());
}

void Player::take_damage(int amount) {
    Creature::take_damage(amount);
    if (is_alive()) {
        hurt_left = server_config().player.hurt_time;
    }
}

EntityAction Player::action() const {
    if (is_unconscious()) {
        return EntityAction::None;
    }
    if (hurt_left > 0) {
        return EntityAction::Hurt;
    }
    if (attacking_left > 0) {
        return EntityAction::Attacking;
    }
    return EntityAction::None;
}

void Player::update(float dt) {
    attack_cooldown_left -= dt;
    attacking_left -= dt;
    hurt_left -= dt;
    if (is_unconscious()) {
        direction = Direction::NONE;  // cae donde está
    }
    advance(dt);
}

bool Player::is_badly_hurt() const {
    return health <= max_health * server_config().player.weak_health;
}

int Player::attack_damage() const {
    return is_badly_hurt() ? static_cast<int>(damage * server_config().player.weak_damage) : damage;
}

void Player::select_inventory_slot(uint8_t slot) { inventory.select_slot(slot); }

InventoryState Player::get_inventory_state() const { return inventory.get_state(); }

SavedPlayer Player::saved() const {
    return SavedPlayer{gold, health, energy, inventory.get_state()};
}

void Player::restore(const SavedPlayer& saved) {
    gold = saved.gold;
    health = saved.health;
    energy = saved.energy;
    inventory.restore(saved.inventory);
}

void Player::set_typing(bool typing) { this->typing = typing; }

bool Player::is_typing() const { return typing; }

uint8_t Player::receive(ItemType item, uint8_t quantity, ItemQuality quality) {
    return inventory.add(item, quantity, quality);
}

int Player::count(ItemType item) const { return inventory.count(item); }

int Player::room_for(ItemType item) const { return inventory.room_for(item); }

void Player::take(ItemType item, int quantity) { inventory.remove(item, quantity); }

uint8_t Player::give_slot(uint8_t slot, Inventory& target) {
    return inventory.move_slot(slot, target);
}

uint8_t Player::take_slot(Inventory& source, uint8_t slot) {
    return source.move_slot(slot, inventory);
}

uint8_t Player::give_slot_to(uint8_t slot, Player& other) {
    return inventory.move_slot(slot, other.inventory);
}

bool Player::split_slot(uint8_t slot) { return inventory.split_slot(slot); }
