#include "server/game/commands/typing_command.h"

#include <gtest/gtest.h>

#include "tests/fake_game_context.h"

TEST(TypingCommandTest, OpeningTheChatMarksThePlayerAsTyping) {
    FakeGameContext context;
    TypingCommand(FakeGameContext::PLAYER_ID, TypingRequest{true}).execute(context);
    EXPECT_TRUE(context.player.is_typing());
}

TEST(TypingCommandTest, ClosingTheChatUnmarksIt) {
    FakeGameContext context;
    TypingCommand(FakeGameContext::PLAYER_ID, TypingRequest{true}).execute(context);
    TypingCommand(FakeGameContext::PLAYER_ID, TypingRequest{false}).execute(context);
    EXPECT_FALSE(context.player.is_typing());
}

TEST(TypingCommandTest, NothingIsSentToAnyone) {
    FakeGameContext context;
    TypingCommand(FakeGameContext::PLAYER_ID, TypingRequest{true}).execute(context);
    EXPECT_TRUE(context.sent.empty());
    EXPECT_TRUE(context.broadcasted.empty());
}

TEST(TypingCommandTest, UnknownPlayerChangesNothing) {
    FakeGameContext context;
    TypingCommand(FakeGameContext::PLAYER_ID + 1, TypingRequest{true}).execute(context);
    EXPECT_FALSE(context.player.is_typing());
}
