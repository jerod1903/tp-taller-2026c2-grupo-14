#include "server/net/receiver.h"

#include <atomic>
#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/logger.h"
#include "common/net/liberror.h"
#include "common/net/socket.h"
#include "server/game/game_event.h"
#include "server/net/server_protocol.h"
#include "tests/fake_game_context.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18645";
// El que el FakeGameContext tiene conectado: así los comandos encolados tienen efecto.
constexpr uint32_t PLAYER_ID = FakeGameContext::PLAYER_ID;

constexpr std::chrono::milliseconds POLL_INTERVAL(10);
constexpr int MAX_POLLS = 200;

class ReceiverTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        server = std::make_unique<ServerProtocol>(listener.accept());
        client = std::make_unique<BaseProtocol>(std::move(client_socket));
        receiver = std::make_unique<Receiver>(*server, inbox, PLAYER_ID, [this] { invites++; });
        receiver->start();
    }

    void TearDown() override {
        try {
            server->shutdown();
        } catch (const LibError&) {}
        receiver.reset();
        Logger::set_level(LogLevel::Info);
    }

    void send_move(int8_t dir_x, int8_t dir_y) {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::Move)));
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(dir_x)));
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(dir_y)));
    }

    // Saca el próximo comando de la inbox y lo ejecuta: el test mira el efecto, no el tipo.
    void execute_next() {
        std::shared_ptr<GameCommand> command = inbox.pop();
        ASSERT_NE(command, nullptr);
        command->execute(context);
    }

    FakeGameContext context;
    Inbox inbox;
    std::unique_ptr<ServerProtocol> server;
    std::unique_ptr<BaseProtocol> client;
    std::unique_ptr<Receiver> receiver;
    // Cuántas veces se llamó a on_invite (desde el hilo del Receiver).
    std::atomic<int> invites{0};
};

TEST_F(ReceiverTest, ForwardsMovesAsMoveCommands) {
    send_move(1, -1);
    execute_next();
    EXPECT_EQ(context.player.get_direction(), Direction::UP_RIGHT);
}

TEST_F(ReceiverTest, InviteCallsOnInviteAndKeepsReading) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::Invite)));
    send_move(1, -1);
    // El Move llegó: el Invite no cortó la conexión ni fue a la inbox
    execute_next();
    EXPECT_EQ(context.player.get_direction(), Direction::UP_RIGHT);
    EXPECT_EQ(invites, 1);
}

TEST_F(ReceiverTest, DayReadyPostsTheOk) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::DayReady)));
    execute_next();
    EXPECT_EQ(context.ready, std::vector<uint32_t>{PLAYER_ID});
}

TEST_F(ReceiverTest, ClientDisconnectPostsLeaveCommand) {
    client.reset();
    execute_next();
    EXPECT_EQ(context.removed, std::vector<uint32_t>{PLAYER_ID});
}

TEST_F(ReceiverTest, LeaveGamePostsLeaveCommandAndEndsTheThread) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::LeaveGame)));
    execute_next();
    EXPECT_EQ(context.removed, std::vector<uint32_t>{PLAYER_ID});
    receiver->join();
    EXPECT_TRUE(receiver->has_left());
}

TEST_F(ReceiverTest, ClientDisconnectIsNotALeave) {
    client.reset();
    inbox.pop();
    receiver->join();
    EXPECT_FALSE(receiver->has_left());
}

TEST_F(ReceiverTest, UnknownOpcodeEndsTheThreadWithoutLeave) {
    ASSERT_TRUE(client->send_u8(0xFF));
    execute_next();
    EXPECT_EQ(context.removed, std::vector<uint32_t>{PLAYER_ID});
    receiver->join();
    EXPECT_FALSE(receiver->has_left());
}

TEST_F(ReceiverTest, MoveCutInTheMiddleEndsTheThreadWithoutLeave) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(GameOp::Move)));
    ASSERT_TRUE(client->send_u8(1));
    client->shutdown();
    execute_next();
    EXPECT_EQ(context.player.get_direction(), Direction::NONE);
    EXPECT_EQ(context.removed, std::vector<uint32_t>{PLAYER_ID});
    receiver->join();
    EXPECT_FALSE(receiver->has_left());
}

TEST_F(ReceiverTest, ClosedInboxEndsTheThread) {
    inbox.close();
    send_move(0, 0);
    for (int i = 0; i < MAX_POLLS && receiver->is_alive(); ++i) {
        std::this_thread::sleep_for(POLL_INTERVAL);
    }
    EXPECT_FALSE(receiver->is_alive());
}

}  // namespace
