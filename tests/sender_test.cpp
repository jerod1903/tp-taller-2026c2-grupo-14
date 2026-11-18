#include "server/net/sender.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "server/game/game_event.h"
#include "server/net/server_protocol.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18644";

constexpr std::chrono::milliseconds POLL_INTERVAL(10);
constexpr uint32_t MAX_POLLS = 200;

class SenderTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        server = std::make_unique<ServerProtocol>(listener.accept());
        client = std::make_unique<BaseProtocol>(std::move(client_socket));
        sender = std::make_unique<Sender>(*server, outbox);
        sender->start();
    }

    void TearDown() override {
        sender.reset();
        Logger::set_level(LogLevel::Info);
    }

    // El primer send a un cliente caído puede entrar en el buffer: falla alguno de los siguientes.
    bool wait_sender_end(bool keep_pushing = false) {
        for (uint32_t i = 0; i < MAX_POLLS && sender->is_alive(); ++i) {
            if (keep_pushing) {
                outbox->try_push(Snapshot{i, 1, 0, {}, {}});
            }
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
        return !sender->is_alive();
    }

    std::shared_ptr<Outbox> outbox = std::make_shared<Outbox>();
    std::unique_ptr<ServerProtocol> server;
    std::unique_ptr<BaseProtocol> client;
    std::unique_ptr<Sender> sender;
};

TEST_F(SenderTest, SendsMessagesInOrder) {
    outbox->push(LocationData{"", 30, 4, 100, 80});
    outbox->push(Snapshot{0, 1, 0, {}, {}});
    EXPECT_EQ(client->recv_u8(),
              std::optional<uint8_t>(static_cast<uint8_t>(GameMsg::LocationData)));
    EXPECT_TRUE(client->recv_string().has_value());
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(30));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(4));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(100));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(80));
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(GameMsg::Snapshot)));
}

TEST_F(SenderTest, SendsPlayerInfo) {
    outbox->push(PlayerInfo{4, "Martu", CharacterAppearance{1, 2, 3, 4, 0}});
    EXPECT_EQ(client->recv_u8(),
              std::optional<uint8_t>(static_cast<uint8_t>(GameMsg::PlayerInfo)));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(4));
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("Martu"));
}

TEST_F(SenderTest, ClosingTheOutboxEndsTheThread) {
    outbox->close();
    EXPECT_TRUE(wait_sender_end());
}

TEST_F(SenderTest, ClientDisconnectEndsTheThread) {
    client.reset();
    EXPECT_TRUE(wait_sender_end(true));
}

}  // namespace
