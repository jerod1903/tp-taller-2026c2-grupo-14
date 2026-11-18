#include "server/game/characters/player.h"

#include <algorithm>
#include <string>
#include <utility>

Player::Player(uint32_t id, const Position& pos, Location& location, std::string name, int health,
               float base_speed, int gold, int energy):
        Creature(id, pos, location, health, base_speed),
        name(std::move(name)),
        gold(gold),
        energy(energy) {}

const std::string& Player::get_name() const { return name; }

int Player::get_gold() const { return gold; }

int Player::get_energy() const { return energy; }

void Player::add_gold(int amount) { gold += amount; }

void Player::spend_energy(int amount) { energy = std::max(0, energy - amount); }

bool Player::is_exhausted() const { return energy == 0; }

void Player::remove_selected(uint8_t quantity) { inventory.remove_selected(quantity); }

ItemType Player::selected_item() const { return inventory.selected_item(); }

void Player::update(float dt) { advance(dt); }

void Player::select_inventory_slot(uint8_t slot) { inventory.select_slot(slot); }

InventoryState Player::get_inventory_state() const { return inventory.get_state(); }

void Player::set_typing(bool typing) { this->typing = typing; }

bool Player::is_typing() const { return typing; }

uint8_t Player::receive(ItemType item, uint8_t quantity, ItemQuality quality) { return inventory.add(item, quantity, quality); }
