#include <cstdint>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "client/network/ClientProtocol.h"
#include "common/lobby_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "server/game/game_registry.h"
#include "server/lobby/lobby_session.h"
#include "server/persistence/in_memory_game_store.h"
#include "tests/test_worlds.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18643";

constexpr uint8_t MAX_PLAYERS = 4;
const std::vector<WorldInfo> WORLDS = {{"granja", "Granja"}, {"pueblo", "Pueblo"}};
const CharacterAppearance APPEARANCE{3, 2, 5, 1, 4};

// Partida multi precargada, de otro usuario que el del login.
constexpr uint32_t MULTI_ID = 7;
constexpr const char* CODE = "ABC234";
constexpr const char* OWNER = "duenio";
const GameRecord MULTI_GAME{
        MULTI_ID, "Multi", "granja", true, MAX_PLAYERS, {{OWNER, "Duenio", APPEARANCE}}, CODE};

// ClientProtocol contra una LobbySession real: si los dos lados no coinciden byte a byte, la
// respuesta se lee mal o la sesion corta la conexion. Las reglas de cada operacion (que nombres
// valen, quien puede entrar) se prueban en lobby_session_test.
class ClientProtocolTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        session = std::make_unique<LobbySession>(listener.accept(), registry);
        session->start();
        client = std::make_unique<ClientProtocol>(std::move(client_socket));
    }

    // La sesion se destruye antes de restaurar el nivel: su thread todavia puede loguear.
    void TearDown() override {
        session.reset();
        Logger::set_level(LogLevel::Info);
    }

    // Declarados antes que la sesion: la sesion usa al registry y se destruye primero.
    InMemoryGameStore store{{MULTI_GAME}};
    GameRegistry registry{test_worlds(WORLDS), store, MAX_PLAYERS};
    std::unique_ptr<ClientProtocol> client;
    std::unique_ptr<LobbySession> session;
};

}  // namespace

TEST_F(ClientProtocolTest, LoginWithValidNameIsAccepted) {
    EXPECT_EQ(client->login("jugador"), LobbyStatus::Ok);
}

TEST_F(ClientProtocolTest, ServerClosingThrowsConnectionError) {
    // Un login rechazado hace que el servidor cierre la conexion.
    ASSERT_EQ(client->login("no vale"), LobbyStatus::InvalidName);
    EXPECT_THROW(client->listWorlds(), LobbyConnectionError);
}

TEST_F(ClientProtocolTest, ListWorldsReturnsServerWorlds) {
    ASSERT_EQ(client->login("jugador"), LobbyStatus::Ok);
    std::vector<WorldInfo> worlds = client->listWorlds();
    ASSERT_EQ(worlds.size(), WORLDS.size());
    for (std::size_t i = 0; i < worlds.size(); ++i) {
        EXPECT_EQ(worlds[i].worldId, WORLDS[i].worldId);
        EXPECT_EQ(worlds[i].name, WORLDS[i].name);
    }
}

TEST_F(ClientProtocolTest, ListMyGamesReturnsAllFields) {
    ASSERT_EQ(client->login(OWNER), LobbyStatus::Ok);
    std::vector<SavedGameInfo> games = client->listMyGames();
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, MULTI_ID);
    EXPECT_EQ(games[0].gameName, "Multi");
    EXPECT_EQ(games[0].worldName, "Granja");
    EXPECT_EQ(games[0].characterName, "Duenio");
    EXPECT_EQ(games[0].appearance.hairStyle, APPEARANCE.hairStyle);
    EXPECT_EQ(games[0].appearance.hairColor, APPEARANCE.hairColor);
    EXPECT_EQ(games[0].appearance.shirt, APPEARANCE.shirt);
    EXPECT_EQ(games[0].appearance.pants, APPEARANCE.pants);
    EXPECT_EQ(games[0].appearance.hat, APPEARANCE.hat);
    EXPECT_TRUE(games[0].isMultiplayer);
}

TEST_F(ClientProtocolTest, CheckInviteCodeReadsOnlyTheStatus) {
    ASSERT_EQ(client->login("jugador"), LobbyStatus::Ok);
    EXPECT_EQ(client->checkInviteCode(CODE), LobbyStatus::Ok);
    // La respuesta es un solo byte: si se leyera de mas, la siguiente operacion se desfasaria.
    EXPECT_EQ(client->listWorlds().size(), WORLDS.size());
}

TEST_F(ClientProtocolTest, CreateGameAddsItToMyGames) {
    ASSERT_EQ(client->login("jugador"), LobbyStatus::Ok);
    GameEnteredResponse response =
            client->createGame({"pueblo", "Mi granja", "Granjero", APPEARANCE});
    ASSERT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.playerId, GameRegistry::OWNER_PLAYER_ID);

    // Después del Ok la conexión ya es de la partida: se consulta al registry.
    std::vector<SavedGameInfo> games = registry.games_of("jugador");
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, response.gameId);
    EXPECT_EQ(games[0].worldName, "Pueblo");
    EXPECT_FALSE(games[0].isMultiplayer);
}

TEST_F(ClientProtocolTest, JoinGameByCodeReturnsItsGameId) {
    ASSERT_EQ(client->login("jugador"), LobbyStatus::Ok);
    GameEnteredResponse response = client->joinGame({CODE, "Invitada", APPEARANCE});
    EXPECT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.gameId, MULTI_ID);
}

TEST_F(ClientProtocolTest, LoadOwnGameIsOk) {
    ASSERT_EQ(client->login(OWNER), LobbyStatus::Ok);
    EXPECT_EQ(client->loadGame(MULTI_ID).status, LobbyStatus::Ok);
}
