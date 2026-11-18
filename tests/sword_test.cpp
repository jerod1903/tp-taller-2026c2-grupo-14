#include "server/game/tools/sword.h"

#include <vector>

#include <gtest/gtest.h>

#include "tests/fake_game_context.h"

namespace {

TEST(SwordTest, AttacksTheClickedCell) {
    FakeGameContext context;
    context.hits = true;
    EXPECT_TRUE(Sword().use(context.player, context, Position{1, 0}));
    EXPECT_EQ(context.attacked, (std::vector<Position>{Position{1, 0}}));
}

TEST(SwordTest, WhenItHitsNothingItIsNotUsed) {
    FakeGameContext context;
    EXPECT_FALSE(Sword().use(context.player, context, Position{1, 0}));
}

}  // namespace
