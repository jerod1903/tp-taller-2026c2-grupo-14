#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "client/network/ClientProtocol.h"
#include "common/base_protocol.h"
#include "common/lobby_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "server/game/game_registry.h"
#include "server/lobby/lobby_session.h"
#include "server/persistence/in_memory_game_store.h"
#include "tests/test_helpers.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* SESSION_PORT = "18643";
constexpr const char* RAW_PORT = "18644";

constexpr uint8_t MAX_PLAYERS = 4;
const std::vector<WorldInfo> WORLDS = {{"granja", "Granja"}, {"pueblo", "Pueblo"}};
const CharacterAppearance APPEARANCE{3, 2, 10, 1, 4};

void expect_same_appearance(const CharacterAppearance& actual,
                            const CharacterAppearance& expected) {
    EXPECT_EQ(actual.hairStyle, expected.hairStyle);
    EXPECT_EQ(actual.hairColor, expected.hairColor);
    EXPECT_EQ(actual.shirt, expected.shirt);
    EXPECT_EQ(actual.pants, expected.pants);
    EXPECT_EQ(actual.hat, expected.hat);
}

// ClientProtocol contra una LobbySession real: las operaciones que el servidor ya implementa.
class ClientProtocolSessionTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        Socket listener(SESSION_PORT);
        Socket client_socket(HOST, SESSION_PORT);
        session = std::make_unique<LobbySession>(listener.accept(), registry);
        session->start();
        client = std::make_unique<ClientProtocol>(std::move(client_socket), user_id);
    }

    void TearDown() override {
        session.reset();
        Logger::set_level(LogLevel::Info);
    }

    const UserId user_id = make_user_id();
    InMemoryGameStore store;
    GameRegistry registry{WORLDS, store, MAX_PLAYERS};
    std::unique_ptr<ClientProtocol> client;
    std::unique_ptr<LobbySession> session;
};

// ClientProtocol contra un servidor a mano: fija el formato de las operaciones que
// ServerProtocol todavia no implementa.
class ClientProtocolWireTest: public ::testing::Test {
protected:
    void SetUp() override {
        Socket listener(RAW_PORT);
        Socket client_socket(HOST, RAW_PORT);
        server = std::make_unique<BaseProtocol>(listener.accept());
        client = std::make_unique<ClientProtocol>(std::move(client_socket), make_user_id());
    }

    void expect_op(LobbyOp expected) {
        std::optional<uint8_t> op = server->recv_u8();
        ASSERT_TRUE(op.has_value());
        EXPECT_EQ(static_cast<LobbyOp>(*op), expected);
    }

    void expect_appearance(const CharacterAppearance& expected) {
        CharacterAppearance received{};
        received.hairStyle = server->recv_u16().value_or(0);
        received.hairColor = server->recv_u8().value_or(0);
        received.shirt = server->recv_u16().value_or(0);
        received.pants = server->recv_u16().value_or(0);
        received.hat = server->recv_u16().value_or(0);
        expect_same_appearance(received, expected);
    }

    std::unique_ptr<BaseProtocol> server;
    std::unique_ptr<ClientProtocol> client;
};

}  // namespace

TEST_F(ClientProtocolSessionTest, LoginWithValidNameIsAccepted) {
    EXPECT_EQ(client->login("martu"), LobbyStatus::Ok);
}

TEST_F(ClientProtocolSessionTest, LoginWithInvalidNameIsRejected) {
    EXPECT_EQ(client->login("no vale"), LobbyStatus::InvalidName);
}

TEST_F(ClientProtocolSessionTest, ListWorldsReturnsServerWorlds) {
    ASSERT_EQ(client->login("martu"), LobbyStatus::Ok);
    std::vector<WorldInfo> worlds = client->listWorlds();
    ASSERT_EQ(worlds.size(), WORLDS.size());
    for (std::size_t i = 0; i < worlds.size(); ++i) {
        EXPECT_EQ(worlds[i].worldId, WORLDS[i].worldId);
        EXPECT_EQ(worlds[i].name, WORLDS[i].name);
    }
}

TEST_F(ClientProtocolSessionTest, ListMyGamesReturnsOwnGames) {
    std::optional<uint32_t> game_id =
            registry.create_game("Mi granja", "granja", true, {user_id, "Martu", APPEARANCE});
    ASSERT_TRUE(game_id.has_value());

    ASSERT_EQ(client->login("martu"), LobbyStatus::Ok);
    std::vector<SavedGameInfo> games = client->listMyGames();
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, *game_id);
    EXPECT_EQ(games[0].gameName, "Mi granja");
    EXPECT_EQ(games[0].characterName, "Martu");
    expect_same_appearance(games[0].appearance, APPEARANCE);
    EXPECT_TRUE(games[0].isMultiplayer);
}

TEST_F(ClientProtocolSessionTest, ServerClosingThrowsConnectionError) {
    // Un login rechazado hace que el servidor cierre la conexion.
    ASSERT_EQ(client->login("no vale"), LobbyStatus::InvalidName);
    EXPECT_THROW(client->listWorlds(), LobbyConnectionError);
}

TEST_F(ClientProtocolWireTest, CheckInviteCodeSendsCodeAndReadsResponse) {
    std::thread fake_server([this]() {
        expect_op(LobbyOp::CheckInviteCode);
        EXPECT_EQ(server->recv_string().value_or(""), "ABC123");
        server->send_u8(static_cast<uint8_t>(LobbyStatus::Ok));
        server->send_u8(1);
        server->send_u32(7);
    });
    CheckInviteCodeResponse response = client->checkInviteCode("ABC123");
    fake_server.join();

    EXPECT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_TRUE(response.alreadyMember);
    EXPECT_EQ(response.gameId, 7u);
}

TEST_F(ClientProtocolWireTest, CreateGameSendsRequestAndReadsGameId) {
    std::thread fake_server([this]() {
        expect_op(LobbyOp::CreateGame);
        EXPECT_EQ(server->recv_string().value_or(""), "granja");
        EXPECT_EQ(server->recv_string().value_or(""), "Mi granja");
        EXPECT_EQ(server->recv_string().value_or(""), "Martu");
        expect_appearance(APPEARANCE);
        server->send_u8(static_cast<uint8_t>(LobbyStatus::Ok));
        server->send_u32(12);
    });
    CreateGameResponse response = client->createGame({"granja", "Mi granja", "Martu", APPEARANCE});
    fake_server.join();

    EXPECT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.gameId, 12u);
}

TEST_F(ClientProtocolWireTest, JoinGameSendsRequestAndReadsStatus) {
    std::thread fake_server([this]() {
        expect_op(LobbyOp::JoinGame);
        EXPECT_EQ(server->recv_u32().value_or(0), 12u);
        EXPECT_EQ(server->recv_string().value_or(""), "Martu");
        expect_appearance(APPEARANCE);
        server->send_u8(static_cast<uint8_t>(LobbyStatus::NameTaken));
    });
    JoinGameResponse response = client->joinGame({12, "Martu", APPEARANCE});
    fake_server.join();

    EXPECT_EQ(response.status, LobbyStatus::NameTaken);
}
