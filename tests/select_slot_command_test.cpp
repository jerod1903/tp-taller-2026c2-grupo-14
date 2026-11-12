#include <variant>

#include <gtest/gtest.h>

#include "server/game/commands/select_slot_command.h"
#include "tests/fake_game_context.h"

TEST(SelectSlotCommandTest, ChangesTheSlotAndSendsTheInventoryToThePlayer) {
    FakeGameContext context;
    SelectSlotCommand command(FakeGameContext::PLAYER_ID, SelectSlotRequest{2});

    command.execute(context);

    EXPECT_EQ(context.player.get_inventory_state().selectedSlot, 2);
    ASSERT_EQ(context.sent.size(), 1u);
    EXPECT_EQ(context.sent[0].first, FakeGameContext::PLAYER_ID);
    ASSERT_TRUE(std::holds_alternative<InventoryState>(context.sent[0].second));
    EXPECT_EQ(std::get<InventoryState>(context.sent[0].second).selectedSlot, 2);
}

TEST(SelectSlotCommandTest, SlotOutOfRangeKeepsTheCurrentOne) {
    FakeGameContext context;
    SelectSlotCommand command(FakeGameContext::PLAYER_ID, SelectSlotRequest{200});

    command.execute(context);

    EXPECT_EQ(context.player.get_inventory_state().selectedSlot, 0);
    // Se reenvia el estado actual: le corrige al cliente cualquier desajuste.
    ASSERT_EQ(context.sent.size(), 1u);
    ASSERT_TRUE(std::holds_alternative<InventoryState>(context.sent[0].second));
    EXPECT_EQ(std::get<InventoryState>(context.sent[0].second).selectedSlot, 0);
}

TEST(SelectSlotCommandTest, UnknownPlayerSendsNothing) {
    FakeGameContext context;
    SelectSlotCommand command(FakeGameContext::PLAYER_ID + 1, SelectSlotRequest{2});

    command.execute(context);

    EXPECT_TRUE(context.sent.empty());
}