#include "server/game/characters/enemies/orc.h"

#include <vector>

#include <gtest/gtest.h>

#include "server/config/server_config.h"
#include "tests/world_helpers.h"

namespace {

constexpr uint32_t SEED = 7;

// El orco en (2, 2) de un nivel de 20x20 sin paredes. `visitors` hace de la lista del nivel.
class OrcTest: public ::testing::Test {
protected:
    TestLocation level{make_grid(20, 20)};
    std::vector<Creature*> visitors;
    Orc orc{1, Position{2, 2}, level, visitors, SEED};

    void kill() { orc.take_damage(server_config().enemies.orc.fighter.health); }
};

TEST_F(OrcTest, HitsAPlayerNextToIt) {
    TestCreature player(level, Position{3, 2});
    visitors = {&player};
    orc.update(0);
    EXPECT_EQ(player.get_health(), 10 - server_config().enemies.orc.fighter.damage);
    EXPECT_EQ(orc.action(), EntityAction::Attacking);
}

TEST_F(OrcTest, TheBodyStaysForAWhileAndThenIsGone) {
    kill();
    EXPECT_EQ(orc.action(), EntityAction::Dead);
    EXPECT_FALSE(orc.is_gone());
    orc.update(CORPSE_TIME / 2);
    EXPECT_FALSE(orc.is_gone());
    orc.update(CORPSE_TIME);
    EXPECT_TRUE(orc.is_gone());
}

TEST_F(OrcTest, DoesNotRevive) {
    kill();
    orc.update(CORPSE_TIME * 10);
    EXPECT_FALSE(orc.is_alive());
}

// Sorteado con muchas semillas: lo que suelta siempre queda entre gold_min y gold_max (o nada), y
// pegarle muerto no da nada
TEST(OrcLootTest, DropsGoldWithinItsRangeOnlyWithTheKillingHit) {
    const ServerConfig::Loot& loot = server_config().enemies.orc.loot;
    TestLocation level{make_grid(5, 5)};
    std::vector<Creature*> visitors;
    int dropped = 0;
    for (uint32_t seed = 0; seed < 200; ++seed) {
        Orc orc{1, Position{2, 2}, level, visitors, seed};
        EXPECT_EQ(orc.take_hit(1), 0);  // no lo mata
        int gold = orc.take_hit(server_config().enemies.orc.fighter.health);
        if (gold > 0) {
            ++dropped;
            EXPECT_GE(gold, loot.gold_min);
            EXPECT_LE(gold, loot.gold_max);
        }
        EXPECT_EQ(orc.take_hit(1), 0);
    }
    EXPECT_GT(dropped, 0);
    EXPECT_LT(dropped, 200);
}

// Con silver_chance deja plata en su celda: con muchas semillas, algunos sí y otros no
TEST(OrcLootTest, SometimesLeavesSilverOnItsCell) {
    std::vector<Creature*> visitors;
    int with_silver = 0;
    for (uint32_t seed = 0; seed < 200; ++seed) {
        TestLocation level{make_grid(5, 5)};
        Orc orc{1, Position{2, 2}, level, visitors, seed};
        orc.take_damage(server_config().enemies.orc.fighter.health);
        auto item = level.get_ground_items().find(Position{2, 2});
        if (item != level.get_ground_items().end()) {
            EXPECT_EQ(item->second, ItemType::Silver);
            ++with_silver;
        }
    }
    EXPECT_GT(with_silver, 0);
    EXPECT_LT(with_silver, 200);
}

}  // namespace
