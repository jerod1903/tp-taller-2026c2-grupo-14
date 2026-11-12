#include <memory>
#include <vector>

#include <gtest/gtest.h>

#include "server/game/commands/join_command.h"
#include "server/game/commands/leave_command.h"
#include "server/game/commands/move_command.h"
#include "tests/fake_game_context.h"

namespace {

constexpr uint32_t PLAYER_ID = FakeGameContext::PLAYER_ID;
constexpr uint32_t UNKNOWN_PLAYER_ID = 99;

class GameCommandsTest: public ::testing::Test {
protected:
    Direction move(int8_t dir_x, int8_t dir_y) {
        MoveCommand(PLAYER_ID, MoveRequest{dir_x, dir_y}).execute(context);
        return context.player.get_direction();
    }

    FakeGameContext context;
};

TEST_F(GameCommandsTest, MoveSetsTheDirection) {
    EXPECT_EQ(move(1, 0), Direction::RIGHT);
    EXPECT_EQ(move(0, -1), Direction::UP);
}

TEST_F(GameCommandsTest, MoveSupportsDiagonals) {
    EXPECT_EQ(move(-1, -1), Direction::UP_LEFT);
    EXPECT_EQ(move(1, 1), Direction::DOWN_RIGHT);
}

TEST_F(GameCommandsTest, MoveZeroStopsThePlayer) {
    move(1, 0);
    EXPECT_EQ(move(0, 0), Direction::NONE);
}

TEST_F(GameCommandsTest, MoveOnlyUsesTheSignOfEachAxis) {
    EXPECT_EQ(move(5, -3), Direction::UP_RIGHT);
}

TEST_F(GameCommandsTest, MoveOfAPlayerNotInTheGameIsIgnored) {
    MoveCommand(UNKNOWN_PLAYER_ID, MoveRequest{1, 0}).execute(context);
    EXPECT_EQ(context.player.get_direction(), Direction::NONE);
}

TEST_F(GameCommandsTest, JoinAddsThePlayerWithItsOutbox) {
    auto outbox = std::make_shared<Outbox>();
    GameMember member{"jero", "Martu", CharacterAppearance{}};
    JoinCommand(PLAYER_ID, member, outbox).execute(context);
    ASSERT_EQ(context.added.size(), 1u);
    EXPECT_EQ(context.added[0].playerId, PLAYER_ID);
    EXPECT_EQ(context.added[0].characterName, "Martu");
    EXPECT_EQ(context.added[0].outbox, outbox);
}

TEST_F(GameCommandsTest, LeaveRemovesThePlayer) {
    LeaveCommand(PLAYER_ID).execute(context);
    EXPECT_EQ(context.removed, std::vector<uint32_t>{PLAYER_ID});
}

}  // namespace
