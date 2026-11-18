#include "server/game/commands/click_command.h"

#include <gtest/gtest.h>

#include "tests/fake_game_context.h"

TEST(ClickCommandTest, TheGreetingGoesToThePlayerWhoClicked) {
    FakeGameContext context;
    context.greeting = ChatMessage{ChatKind::Merchant, "Marnie", "¡Hola!"};
    ClickCommand(FakeGameContext::PLAYER_ID, ClickRequest{16, 8}).execute(context);
    ASSERT_EQ(context.clicked.size(), 1u);
    EXPECT_EQ(context.clicked[0], (Position{16, 8}));
    ASSERT_EQ(context.sent.size(), 1u);
    EXPECT_EQ(context.sent[0].first, FakeGameContext::PLAYER_ID);
}

TEST(ClickCommandTest, ClickingWhereThereIsNobodySendsNothing) {
    FakeGameContext context;
    ClickCommand(FakeGameContext::PLAYER_ID, ClickRequest{0, 0}).execute(context);
    EXPECT_EQ(context.clicked.size(), 1u);
    EXPECT_TRUE(context.sent.empty());
}

TEST(ClickCommandTest, AnUnknownPlayerDoesNothing) {
    FakeGameContext context;
    context.greeting = ChatMessage{ChatKind::Merchant, "Marnie", "¡Hola!"};
    ClickCommand(FakeGameContext::PLAYER_ID + 1, ClickRequest{16, 8}).execute(context);
    EXPECT_TRUE(context.clicked.empty());
    EXPECT_TRUE(context.sent.empty());
}
