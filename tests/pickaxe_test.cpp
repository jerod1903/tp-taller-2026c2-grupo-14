#include "server/game/tools/pickaxe.h"

#include <algorithm>

#include <gtest/gtest.h>

#include "common/farm_dto.h"
#include "server/config/server_config.h"
#include "server/game/commands/click_command.h"
#include "server/game/world/dungeon/dungeon_level.h"
#include "tests/fake_game_context.h"
#include "tests/world_helpers.h"

namespace {

uint32_t find_coal_seed(FakeGameContext& context) {
    for (uint32_t seed = 1; seed < 200; ++seed) {
        DungeonLevel candidate{"level", make_grid(20, 20), 0, seed};
        candidate.add_portal(Portal{Position{0, 0}, &context.location, Point{0.5f, 0.5f}});
        candidate.set_visitors({&context.player});
        for (const auto& [pos, node]: candidate.get_ores()) {
            if (node.ore == ItemType::Coal) {
                return seed;
            }
        }
    }
    return 7;
}

class PickaxeTest: public ::testing::Test {
protected:
    PickaxeTest() {
        dungeon.add_portal(Portal{Position{0, 0}, &context.location, Point{0.5f, 0.5f}});
        // Entramos al nivel para que genere los minerales
        dungeon.set_visitors({&context.player});
        coal_pos = find_coal();
        // Paramos al jugador una celda arriba del carbón mirando hacia abajo
        player_pos = Position{coal_pos.x, coal_pos.y - 1};
        context.player.relocate(dungeon, player_pos);
        context.player.set_direction(Direction::DOWN);
    }

    Position find_coal() const {
        for (const auto& [pos, node]: dungeon.get_ores()) {
            if (node.ore == ItemType::Coal) {
                return pos;
            }
        }
        ADD_FAILURE() << "No se encontró carbón en el nivel";
        return Position{5, 5};
    }

    FakeGameContext context;
    DungeonLevel dungeon{"level", make_grid(20, 20), 0, find_coal_seed(context)};
    Pickaxe pickaxe;
    Position coal_pos{0, 0};
    Position player_pos{0, 0};
};

TEST_F(PickaxeTest, StrikingOreAheadConsumesEnergyAndGivesItem) {
    const int energy_before = context.player.get_energy();

    ClickCommand(FakeGameContext::PLAYER_ID,
                 ClickRequest{static_cast<uint16_t>(coal_pos.x), static_cast<uint16_t>(coal_pos.y)})
            .execute(context);
    EXPECT_EQ(context.player.get_energy(), energy_before - server_config().energy.mine);
    EXPECT_EQ(context.player.count(ItemType::Coal), 1);
    EXPECT_EQ(dungeon.ore_at(coal_pos), nullptr);
}

TEST_F(PickaxeTest, ClickingAnotherCellOrNonDungeonLocationDoesNothing) {
    const int energy_before = context.player.get_energy();
    const Position other{player_pos.x + 1, player_pos.y};

    // Click en otra celda distinta a la de adelante
    EXPECT_FALSE(pickaxe.use(context.player, context, other));
    EXPECT_EQ(context.player.get_energy(), energy_before);
    EXPECT_NE(dungeon.ore_at(coal_pos), nullptr);

    // En una Location que no es DungeonLevel
    context.player.relocate(context.location, Position{1, 1});
    context.player.set_direction(Direction::DOWN);
    EXPECT_FALSE(pickaxe.use(context.player, context, Position{1, 2}));
    EXPECT_EQ(context.player.get_energy(), energy_before);
}

TEST_F(PickaxeTest, WhenInventoryIsFullItemDropsOnLevelFloor) {
    // Llenamos todos los slots del inventario con picos hasta que receive devuelva sobrante > 0
    while (context.player.receive(ItemType::Pickaxe, 1) == 0) {}

    EXPECT_TRUE(pickaxe.use(context.player, context, coal_pos));
    EXPECT_EQ(context.player.count(ItemType::Coal), 0);

    const auto& ground = dungeon.get_ground_items();
    auto it = ground.find(coal_pos);
    ASSERT_NE(it, ground.end());
    EXPECT_EQ(it->second, ItemType::Coal);
}

TEST_F(PickaxeTest, MiningSilverSpawnsOrcPackOnLowRoll) {
    const int silver_hits = server_config().mining.silver.hits;
    const float chance = server_config().mining.silver_orc_chance;

    for (uint32_t seed = 1; seed < 200; ++seed) {
        FakeGameContext iter_context;
        DungeonLevel probe{"level", make_grid(20, 20), 4, seed};
        probe.add_portal(Portal{Position{0, 0}, &iter_context.location, Point{0.5f, 0.5f}});
        probe.set_visitors({&iter_context.player});

        auto it = std::find_if(
                probe.get_ores().begin(), probe.get_ores().end(),
                [](const auto& entry) { return entry.second.ore == ItemType::Silver; });
        if (it == probe.get_ores().end()) {
            continue;
        }
        const Position silver_pos = it->first;
        const size_t enemies_before = probe.get_enemies().size();

        iter_context.player.relocate(probe, Position{silver_pos.x, silver_pos.y - 1});
        iter_context.player.set_direction(Direction::DOWN);

        for (int i = 0; i < silver_hits; ++i) {
            pickaxe.use(iter_context.player, iter_context, silver_pos);
        }
        if (probe.get_enemies().size() > enemies_before) {
            EXPECT_EQ(probe.ore_at(silver_pos), nullptr);
            EXPECT_EQ(iter_context.player.count(ItemType::Silver), 1);
            EXPECT_GE(probe.get_enemies().size(),
                      enemies_before + static_cast<size_t>(server_config().enemies.orc.min_group));
            EXPECT_EQ(probe.get_enemies().back()->entity_type(), EntityType::Orc);
            return;
        }
    }
    FAIL() << "Ninguna semilla disparó la manada de orcos al minar plata (chance=" << chance << ")";
}

}  // namespace