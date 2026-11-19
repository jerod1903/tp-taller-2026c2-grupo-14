#include "server/game/commands/chat_command.h"

#include <string>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "tests/fake_game_context.h"

namespace {

constexpr uint32_t DIPA_ID = 9;

ChatMessage chat_of(const GameMessage& message) {
    EXPECT_TRUE(std::holds_alternative<ChatMessage>(message));
    return std::get<ChatMessage>(message);
}

void say(FakeGameContext& context, const std::string& text) {
    ChatCommand(FakeGameContext::PLAYER_ID, ChatRequest{text}).execute(context);
}

}  // namespace

TEST(ChatCommandTest, APlainMessageGoesToEveryone) {
    FakeGameContext context;
    say(context, "Hola, gente!");

    ASSERT_EQ(context.broadcasted.size(), 1u);
    EXPECT_EQ(chat_of(context.broadcasted[0]),
              (ChatMessage{ChatKind::Public, "jero", "Hola, gente!"}));
    EXPECT_TRUE(context.sent.empty());
}

TEST(ChatCommandTest, AnArrobaGoesOnlyToThatPlayerAndACopyToTheAuthor) {
    FakeGameContext context;
    context.connected["dipa"] = DIPA_ID;
    say(context, "@dipa vamos a las mazmorras?");

    const ChatMessage expected{ChatKind::Private, "jero", "vamos a las mazmorras?"};
    EXPECT_TRUE(context.broadcasted.empty());
    ASSERT_EQ(context.sent.size(), 2u);
    EXPECT_EQ(context.sent[0].first, DIPA_ID);
    EXPECT_EQ(chat_of(context.sent[0].second), expected);
    EXPECT_EQ(context.sent[1].first, FakeGameContext::PLAYER_ID);
    EXPECT_EQ(chat_of(context.sent[1].second), expected);
}

TEST(ChatCommandTest, APrivateMessageToSomeoneOfflineWarnsTheAuthor) {
    FakeGameContext context;
    say(context, "@dipa hola");

    ASSERT_EQ(context.sent.size(), 1u);
    EXPECT_EQ(context.sent[0].first, FakeGameContext::PLAYER_ID);
    EXPECT_EQ(chat_of(context.sent[0].second),
              (ChatMessage{ChatKind::System, "", "dipa no está conectado"}));
}

TEST(ChatCommandTest, AMessageToOneselfArrivesOnce) {
    FakeGameContext context;
    say(context, "@jero recordatorio");

    ASSERT_EQ(context.sent.size(), 1u);
    EXPECT_EQ(context.sent[0].first, FakeGameContext::PLAYER_ID);
}

TEST(ChatCommandTest, BlankOrEmptyMessagesAreNotSent) {
    FakeGameContext context;
    context.connected["dipa"] = DIPA_ID;
    say(context, "   ");
    say(context, "@dipa   ");

    EXPECT_TRUE(context.broadcasted.empty());
    EXPECT_TRUE(context.sent.empty());
}

TEST(ChatCommandTest, SpacesAroundAreRemoved) {
    FakeGameContext context;
    say(context, "   hola   ");

    ASSERT_EQ(context.broadcasted.size(), 1u);
    EXPECT_EQ(chat_of(context.broadcasted[0]).text, "hola");
}

TEST(ChatCommandTest, LongMessagesAreCutWithoutBreakingALetter) {
    FakeGameContext context;
    // 79 letras y una con tilde (2 bytes): el corte en 80 caeria en la mitad de la "á"
    say(context, std::string(MAX_CHAT_LENGTH - 1, 'a') + "á y mas");

    ASSERT_EQ(context.broadcasted.size(), 1u);
    EXPECT_EQ(chat_of(context.broadcasted[0]).text, std::string(MAX_CHAT_LENGTH - 1, 'a'));
}

TEST(ChatCommandTest, UnknownPlayerSendsNothing) {
    FakeGameContext context;
    ChatCommand(FakeGameContext::PLAYER_ID + 1, ChatRequest{"hola"}).execute(context);

    EXPECT_TRUE(context.broadcasted.empty());
    EXPECT_TRUE(context.sent.empty());
}

TEST(ChatCommandTest, ASlashGoesToTheMerchantAndNotToEveryone) {
    FakeGameContext context;
    say(context, "/comprar vaca 2");
    EXPECT_TRUE(context.broadcasted.empty());
    EXPECT_EQ(context.commands, std::vector<std::string>{"/comprar vaca 2"});
    ASSERT_EQ(context.sent.size(), 1u);
    EXPECT_EQ(context.sent[0].first, FakeGameContext::PLAYER_ID);
    EXPECT_EQ(chat_of(context.sent[0].second), context.trade_reply.message);
}

TEST(ChatCommandTest, IfTheTradeChangedTheInventoryItIsSentToo) {
    FakeGameContext context;
    context.trade_reply.inventory_changed = true;
    say(context, "/vender leche");
    ASSERT_EQ(context.sent.size(), 2u);
    EXPECT_TRUE(std::holds_alternative<InventoryState>(context.sent[1].second));
}

TEST(ChatCommandTest, TheSleepCheatEndsTheDayWithoutSayingAnything) {
    FakeGameContext context;
    say(context, "$sleep");
    EXPECT_EQ(context.days_ended, 1);
    EXPECT_TRUE(context.broadcasted.empty());
}

TEST(ChatCommandTest, TheSleepCheatTakesNoPayload) {
    FakeGameContext context;
    say(context, "$sleep 3");
    EXPECT_EQ(context.days_ended, 0);
    EXPECT_EQ(context.sent.size(), 1u);
}
