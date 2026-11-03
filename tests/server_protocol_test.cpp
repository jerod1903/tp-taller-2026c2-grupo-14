#include "server/net/server_protocol.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/net/socket.h"
#include "common/operations.h"
#include "common/requests.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18643";

// Cada mensaje entra en el buffer del socket: no hace falta un thread del otro lado.
class ServerProtocolTest: public ::testing::Test {
protected:
    void SetUp() override {
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        server = std::make_unique<ServerProtocol>(listener.accept());
        client = std::make_unique<BaseProtocol>(std::move(client_socket));
    }

    void expect_msg(GameMsg expected) {
        std::optional<uint8_t> tag = client->recv_u8();
        ASSERT_TRUE(tag.has_value());
        EXPECT_EQ(static_cast<GameMsg>(*tag), expected);
    }

    std::unique_ptr<ServerProtocol> server;
    std::unique_ptr<BaseProtocol> client;
};

TEST_F(ServerProtocolTest, LocationDataGoesWithItsTagAndWorldId) {
    ASSERT_TRUE(server->send_location_data(LocationData{"granja"}));
    expect_msg(GameMsg::LocationData);
    EXPECT_EQ(client->recv_string(), std::optional<std::string>("granja"));
}

TEST_F(ServerProtocolTest, SnapshotGoesWithTickAndConnectedPlayers) {
    ASSERT_TRUE(server->send_snapshot(Snapshot{42, {0, 3}}));
    expect_msg(GameMsg::Snapshot);
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(42));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(2));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(0));
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(3));
}

TEST_F(ServerProtocolTest, GameEndedGoesWithItsTagAndReason) {
    ASSERT_TRUE(server->send_game_ended(GameEnded{EndReason::Left}));
    expect_msg(GameMsg::GameEnded);
    EXPECT_EQ(client->recv_u8(), std::optional<uint8_t>(static_cast<uint8_t>(EndReason::Left)));
}

TEST_F(ServerProtocolTest, EmptySnapshotHasNoPlayers) {
    ASSERT_TRUE(server->send_snapshot(Snapshot{0, {}}));
    expect_msg(GameMsg::Snapshot);
    EXPECT_EQ(client->recv_u32(), std::optional<uint32_t>(0));
    EXPECT_EQ(client->recv_u16(), std::optional<uint16_t>(0));
}

TEST_F(ServerProtocolTest, RecvRequestReadsOperationAndDirection) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(Operation::Move)));
    ASSERT_TRUE(client->send_u8(1));
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(-1)));
    std::optional<Request> request = server->recv_request();
    ASSERT_TRUE(request.has_value());
    EXPECT_EQ(request->tipo, Operation::Move);
    EXPECT_EQ(request->dir_x, 1);
    EXPECT_EQ(request->dir_y, -1);
}

TEST_F(ServerProtocolTest, RecvRequestCutInTheMiddleIsNullopt) {
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(Operation::Move)));
    client->shutdown();
    EXPECT_FALSE(server->recv_request().has_value());
}

}  // namespace
