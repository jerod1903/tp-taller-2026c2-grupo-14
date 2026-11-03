#include "server/net/receiver.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <thread>
#include <utility>
#include <variant>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/logger.h"
#include "common/net/liberror.h"
#include "common/net/socket.h"
#include "common/operations.h"
#include "server/game/game_event.h"
#include "server/net/server_protocol.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18645";
constexpr uint32_t PLAYER_ID = 3;

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
        receiver = std::make_unique<Receiver>(*server, inbox, PLAYER_ID);
        receiver->start();
    }

    void TearDown() override {
        try {
            server->shutdown();
        } catch (const LibError&) {}
        receiver.reset();
        Logger::set_level(LogLevel::Info);
    }

    void send_move(uint8_t dir_x, uint8_t dir_y) {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(Operation::Move)));
        ASSERT_TRUE(client->send_u8(dir_x));
        ASSERT_TRUE(client->send_u8(dir_y));
    }

    Inbox inbox;
    std::unique_ptr<ServerProtocol> server;
    std::unique_ptr<BaseProtocol> client;
    std::unique_ptr<Receiver> receiver;
};

TEST_F(ReceiverTest, ForwardsRequestsAsPlayerActions) {
    send_move(1, 2);
    GameEvent event = inbox.pop();
    const PlayerAction* action = std::get_if<PlayerAction>(&event);
    ASSERT_NE(action, nullptr);
    EXPECT_EQ(action->playerId, PLAYER_ID);
    EXPECT_EQ(action->request.tipo, Operation::Move);
    EXPECT_EQ(action->request.dir_x, 1);
    EXPECT_EQ(action->request.dir_y, 2);
}

TEST_F(ReceiverTest, ClientDisconnectPostsPlayerLeft) {
    client.reset();
    GameEvent event = inbox.pop();
    const PlayerLeft* left = std::get_if<PlayerLeft>(&event);
    ASSERT_NE(left, nullptr);
    EXPECT_EQ(left->playerId, PLAYER_ID);
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
