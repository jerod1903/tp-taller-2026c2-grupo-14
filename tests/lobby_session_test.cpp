#include "server/lobby_session.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "common/base_protocol.h"
#include "common/lobby_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "server/consts.h"
#include "server/game_registry.h"
#include "server/in_memory_game_store.h"
#include "tests/test_helpers.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18642";

constexpr uint8_t UNKNOWN_OP = 0xFF;
constexpr std::chrono::milliseconds POLL_INTERVAL(10);
constexpr int MAX_POLLS = 200;

constexpr uint8_t MAX_PLAYERS = 4;
const std::vector<WorldInfo> WORLDS = {{"granja", "Granja"}, {"pueblo", "Pueblo"}};
const CharacterAppearance APPEARANCE{3, 2, 10, 1, 4};

class LobbySessionTest: public ::testing::Test {
protected:
    // El test hace de cliente; la LobbySession corre en su propio thread del otro lado del socket.
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        session = std::make_unique<LobbySession>(listener.accept(), registry);
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
        send_login(PROTOCOL_VERSION, make_user_id(), username);
    }

    void expect_status(LobbyStatus expected) {
        std::optional<uint8_t> status = client->recv_u8();
        ASSERT_TRUE(status.has_value());
        EXPECT_EQ(static_cast<LobbyStatus>(*status), expected);
    }

    void send_op(LobbyOp op) { ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(op))); }

    // Decodifica del lado del cliente lo que manda ServerProtocol::send_world_list.
    std::vector<WorldInfo> recv_world_list() {
        std::vector<WorldInfo> worlds;
        std::optional<uint16_t> count = client->recv_u16();
        EXPECT_TRUE(count.has_value());
        for (uint16_t i = 0; count && i < *count; ++i) {
            std::optional<std::string> id = client->recv_string();
            std::optional<std::string> name = client->recv_string();
            EXPECT_TRUE(id && name);
            worlds.push_back(WorldInfo{id.value_or(""), name.value_or("")});
        }
        return worlds;
    }

    // Decodifica del lado del cliente lo que manda ServerProtocol::send_saved_game_list.
    std::vector<SavedGameInfo> recv_saved_game_list() {
        std::vector<SavedGameInfo> games;
        std::optional<uint16_t> count = client->recv_u16();
        EXPECT_TRUE(count.has_value());
        for (uint16_t i = 0; count && i < *count; ++i) {
            SavedGameInfo game{};
            game.gameId = client->recv_u32().value_or(0);
            game.gameName = client->recv_string().value_or("");
            game.characterName = client->recv_string().value_or("");
            game.appearance.hairStyle = client->recv_u16().value_or(0);
            game.appearance.hairColor = client->recv_u8().value_or(0);
            game.appearance.shirt = client->recv_u16().value_or(0);
            game.appearance.pants = client->recv_u16().value_or(0);
            game.appearance.hat = client->recv_u16().value_or(0);
            game.isMultiplayer = client->recv_u8().value_or(0) != 0;
            game.memberCount = client->recv_u8().value_or(0);
            game.connectedCount = client->recv_u8().value_or(0);
            game.maxPlayers = client->recv_u8().value_or(0);
            games.push_back(game);
        }
        return games;
    }

    void expect_server_closed() { EXPECT_FALSE(client->recv_u8().has_value()); }

    bool wait_session_end() {
        for (int i = 0; i < MAX_POLLS && session->is_alive(); ++i) {
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
        return !session->is_alive();
    }

    // Declarados antes que la sesión: la sesión usa al registry y se destruye primero.
    InMemoryGameStore store;
    GameRegistry registry{WORLDS, store, MAX_PLAYERS};
    std::unique_ptr<BaseProtocol> client;
    std::unique_ptr<LobbySession> session;
};

TEST_F(LobbySessionTest, ValidLoginRespondsOk) {
    send_login("jero_14");
    expect_status(LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, ValidLoginKeepsSessionWaitingForOperations) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::ListWorlds);
    EXPECT_EQ(recv_world_list().size(), WORLDS.size());
    session.reset();
    expect_server_closed();
}

TEST_F(LobbySessionTest, AcceptsUsernameWithMaxLength) {
    send_login(std::string(ServerConstants::MAX_NAME_LEN, 'a'));
    expect_status(LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, IncompatibleVersionRespondsProtocolMismatchAndCloses) {
    send_login(PROTOCOL_VERSION + 1, make_user_id(), "jero");
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
    UserId user_id = make_user_id();
    user_id[6] = static_cast<uint8_t>(0x10 | (user_id[6] & 0x0F));
    send_login(PROTOCOL_VERSION, user_id, "jero");
    expect_status(LobbyStatus::InvalidName);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UserIdWithWrongVariantRespondsInvalidName) {
    UserId user_id = make_user_id();
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
    UserId user_id = make_user_id();
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

TEST_F(LobbySessionTest, ListWorldsBeforeLoginClosesConnection) {
    send_op(LobbyOp::ListWorlds);
    expect_server_closed();
}

TEST_F(LobbySessionTest, ListMyGamesBeforeLoginClosesConnection) {
    send_op(LobbyOp::ListMyGames);
    expect_server_closed();
}

// Alcanza con el opcode: el servidor cierra sin leer el resto, y mandarlo completo sería una
// carrera contra ese cierre.
TEST_F(LobbySessionTest, SecondLoginClosesConnection) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::Login);
    expect_server_closed();
}

TEST_F(LobbySessionTest, UnknownOperationAfterLoginClosesConnection) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    ASSERT_TRUE(client->send_u8(UNKNOWN_OP));
    expect_server_closed();
}

TEST_F(LobbySessionTest, ListWorldsReturnsServerWorlds) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::ListWorlds);
    std::vector<WorldInfo> worlds = recv_world_list();
    ASSERT_EQ(worlds.size(), WORLDS.size());
    for (std::size_t i = 0; i < worlds.size(); ++i) {
        EXPECT_EQ(worlds[i].worldId, WORLDS[i].worldId);
        EXPECT_EQ(worlds[i].name, WORLDS[i].name);
    }
}

TEST_F(LobbySessionTest, ListMyGamesWithoutGamesReturnsEmptyList) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::ListMyGames);
    EXPECT_TRUE(recv_saved_game_list().empty());
}

TEST_F(LobbySessionTest, ListMyGamesReturnsAllFieldsOfOwnGamesOnly) {
    UserId me = make_user_id();
    std::optional<uint32_t> mine =
            registry.create_game("Mi granja", "granja", true, GameMember{me, "Martu", APPEARANCE});
    registry.create_game("Ajena", "pueblo", true,
                         GameMember{make_user_id(0x10), "Otro", APPEARANCE});

    send_login(PROTOCOL_VERSION, me, "jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::ListMyGames);
    std::vector<SavedGameInfo> games = recv_saved_game_list();
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, *mine);
    EXPECT_EQ(games[0].gameName, "Mi granja");
    EXPECT_EQ(games[0].characterName, "Martu");
    EXPECT_EQ(games[0].appearance.hairStyle, APPEARANCE.hairStyle);
    EXPECT_EQ(games[0].appearance.hairColor, APPEARANCE.hairColor);
    EXPECT_EQ(games[0].appearance.shirt, APPEARANCE.shirt);
    EXPECT_EQ(games[0].appearance.pants, APPEARANCE.pants);
    EXPECT_EQ(games[0].appearance.hat, APPEARANCE.hat);
    EXPECT_TRUE(games[0].isMultiplayer);
    EXPECT_EQ(games[0].memberCount, 1);
    EXPECT_EQ(games[0].connectedCount, 0);
    EXPECT_EQ(games[0].maxPlayers, MAX_PLAYERS);
}

TEST_F(LobbySessionTest, SeveralOperationsInTheSameConnection) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::ListWorlds);
    EXPECT_EQ(recv_world_list().size(), WORLDS.size());
    send_op(LobbyOp::ListMyGames);
    EXPECT_TRUE(recv_saved_game_list().empty());
    send_op(LobbyOp::ListWorlds);
    EXPECT_EQ(recv_world_list().size(), WORLDS.size());
}

}  // namespace
