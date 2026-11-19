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

}  // namespace
