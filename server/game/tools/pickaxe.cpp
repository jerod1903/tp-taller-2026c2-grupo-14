#include "server/game/tools/pickaxe.h"

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/commands/game_context.h"
#include "server/game/world/dungeon/dungeon_level.h"
#include "server/game/world/dungeon/spawners/orc_spawner.h"
#include <random>

bool Pickaxe::use(Player& player, GameContext& context, const Position& cell) const {
    auto* level = dynamic_cast<DungeonLevel*>(&player.get_location());
    if (level == nullptr || cell != player.facing_cell()) {
        return false;
    }
    DungeonLevel::StrikeResult result = level->strike_ore(cell);
    if (result.strike == DungeonLevel::Strike::Mined) {
    if (player.receive(result.ore, 1) > 0) {
        level->drop(cell, result.ore);
    } else {
        context.sessions().send_to(player.get_id(), player.get_inventory_state());
    }
    if (result.ore == ItemType::Silver) {
        static thread_local std::mt19937 rng{std::random_device{}()};
        if (std::uniform_real_distribution<float>(0.0f, 1.0f)(rng) <
            server_config().mining.silver_orc_chance) {
            OrcSpawner().spawn(*level);
        }
    }
    }
    return true;
}

int Pickaxe::energy_cost() const { return server_config().energy.mine; }