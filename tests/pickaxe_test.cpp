#include "server/game/tools/pickaxe.h"

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "server/game/world/dungeon/dungeon_level.h"
#include "tests/fake_game_context.h"
#include "tests/world_helpers.h"

namespace {

class PickaxeTest: public ::testing::Test {
protected:
    PickaxeTest() {
        dungeon.add_portal(Portal{Position{0, 0}, &context.location, Point{0.5f, 0.5f}});
        // Entramos al nivel para que genere los minerales
        dungeon.set_visitors({&context.player});
        coal_pos = find_coal();
        // Paramos al jugador una celda arriba del carbón mirando hacia abajo
        player_pos = Position{coal_pos.x, coal_pos.y - 1};
        context.player.set_location(dungeon, player_pos);
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
    DungeonLevel dungeon{"level", make_grid(20, 20), 0, 7};
    Pickaxe pickaxe;
    Position coal_pos{0, 0};
    Position player_pos{0, 0};
};

TEST_F(PickaxeTest, StrikingOreAheadConsumesEnergyAndGivesItem) {
    const int energy_before = context.player.get_energy();

    EXPECT_TRUE(pickaxe.use(context.player, context, coal_pos));
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
    context.player.set_location(context.location, Position{1, 1});
    context.player.set_direction(Direction::DOWN);
    EXPECT_FALSE(pickaxe.use(context.player, context, Position{1, 2}));
    EXPECT_EQ(context.player.get_energy(), energy_before);
}

TEST_F(PickaxeTest, WhenInventoryIsFullItemDropsOnLevelFloor) {
    // Llenamos todos los slots del inventario hasta que receive devuelva sobrante > 0
    while (context.player.receive(ItemType::Seed, 1) == 0) {}

    EXPECT_TRUE(pickaxe.use(context.player, context, coal_pos));
    EXPECT_EQ(context.player.count(ItemType::Coal), 0);
    ASSERT_FALSE(dungeon.get_drops().empty());
    EXPECT_EQ(dungeon.get_drops().front().item, ItemType::Coal);
    EXPECT_EQ(dungeon.get_drops().front().pos, coal_pos);
}

}  // namespace