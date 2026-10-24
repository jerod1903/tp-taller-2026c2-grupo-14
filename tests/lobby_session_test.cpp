#include "server/lobby_session.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/lobby_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "server/consts.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18642";

constexpr uint8_t UNKNOWN_OP = 0xFF;
constexpr std::chrono::milliseconds POLL_INTERVAL(10);
constexpr int MAX_POLLS = 200;

UserId make_uuid_v4() {
    UserId id{};
    for (std::size_t i = 0; i < id.size(); ++i) {
        id[i] = static_cast<uint8_t>(0xA0 + i);
    }
    id[6] = static_cast<uint8_t>(0x40 | (id[6] & 0x0F));
    id[8] = static_cast<uint8_t>(0x80 | (id[8] & 0x3F));
    return id;
}

class LobbySessionTest: public ::testing::Test {
protected:
    // El test hace de cliente; la LobbySession corre en su propio thread del otro lado del socket.
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        session = std::make_unique<LobbySession>(listener.accept());
        session->start();
        client = std::make_unique<BaseProtocol>(std::move(client_socket));
    }

    // La sesión se destruye antes de restaurar el nivel: su thread todavía puede loguear.
    void TearDown() override {
        session.reset();
        Logger::set_level(LogLevel::Info);
    }

    void send_login(uint8_t version, const UserId& user_id, const std::string& username) {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(LobbyOp::Login)));
        ASSERT_TRUE(client->send_u8(version));
        ASSERT_TRUE(client->send_bytes(user_id.data(), user_id.size()));
        ASSERT_TRUE(client->send_string(username));
    }

    void send_login(const std::string& username) {
        send_login(PROTOCOL_VERSION, make_uuid_v4(), username);
    }

    void expect_status(LobbyStatus expected) {
        std::optional<uint8_t> status = client->recv_u8();
        ASSERT_TRUE(status.has_value());
        EXPECT_EQ(static_cast<LobbyStatus>(*status), expected);
    }

    void expect_server_closed() { EXPECT_FALSE(client->recv_u8().has_value()); }

    bool wait_session_end() {
        for (int i = 0; i < MAX_POLLS && session->is_alive(); ++i) {
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
        return !session->is_alive();
    }

    std::unique_ptr<BaseProtocol> client;
    std::unique_ptr<LobbySession> session;
};

TEST_F(LobbySessionTest, ValidLoginRespondsOk) {
    send_login("jero_14");
    expect_status(LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, ValidLoginKeepsConnectionUntilSessionIsDestroyed) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    ASSERT_TRUE(wait_session_end());
    session.reset();
    expect_server_closed();
}

TEST_F(LobbySessionTest, AcceptsUsernameWithMaxLength) {
    send_login(std::string(ServerConstants::MAX_NAME_LEN, 'a'));
    expect_status(LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, IncompatibleVersionRespondsProtocolMismatchAndCloses) {
    send_login(PROTOCOL_VERSION + 1, make_uuid_v4(), "jero");
    expect_status(LobbyStatus::ProtocolMismatch);
    expect_server_closed();
}

TEST_F(LobbySessionTest, VersionIsCheckedBeforeIdentity) {
    send_login(PROTOCOL_VERSION + 1, UserId{}, "");
    expect_status(LobbyStatus::ProtocolMismatch);
    expect_server_closed();
}

TEST_F(LobbySessionTest, EmptyUsernameRespondsInvalidNameAndCloses) {
    send_login("");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, TooLongUsernameRespondsInvalidNameAndCloses) {
    send_login(std::string(ServerConstants::MAX_NAME_LEN + 1, 'a'));
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UsernameWithSpaceRespondsInvalidNameAndCloses) {
    send_login("jero 14");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UsernameWithNonAsciiRespondsInvalidNameAndCloses) {
    send_login("ñandú");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, NilUserIdRespondsInvalidNameAndCloses) {
    send_login(PROTOCOL_VERSION, UserId{}, "jero");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UserIdWithWrongVersionNibbleRespondsInvalidName) {
    UserId user_id = make_uuid_v4();
    user_id[6] = static_cast<uint8_t>(0x10 | (user_id[6] & 0x0F));
    send_login(PROTOCOL_VERSION, user_id, "jero");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UserIdWithWrongVariantRespondsInvalidName) {
    UserId user_id = make_uuid_v4();
    user_id[8] = static_cast<uint8_t>(0xC0 | (user_id[8] & 0x3F));
    send_login(PROTOCOL_VERSION, user_id, "jero");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UnknownOperationClosesWithoutResponse) {
    ASSERT_TRUE(client->send_u8(UNKNOWN_OP));
    expect_server_closed();
}

TEST_F(LobbySessionTest, ClientDisconnectingMidLoginEndsSession) {
    UserId user_id = make_uuid_v4();
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(LobbyOp::Login)));
    ASSERT_TRUE(client->send_u8(PROTOCOL_VERSION));
    ASSERT_TRUE(client->send_bytes(user_id.data(), user_id.size() / 2));
    client->shutdown();
    EXPECT_TRUE(wait_session_end());
}

TEST_F(LobbySessionTest, DestroyingSessionUnblocksPendingLogin) {
    session.reset();
    expect_server_closed();
}

}  // namespace
