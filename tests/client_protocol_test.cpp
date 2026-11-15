#include <cstdint>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "client/network/ClientProtocol.h"
#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/lobby_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "server/game/game_registry.h"
#include "server/lobby/lobby_session.h"
#include "server/net/server_protocol.h"
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

TEST_F(ClientProtocolTest, EnableMultiplayerReturnsTheInviteCode) {
    ASSERT_EQ(client->login(OWNER), LobbyStatus::Ok);
    EnableMultiplayerResponse response = client->enableMultiplayer(MULTI_ID);
    EXPECT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.inviteCode, CODE);
}

TEST_F(ClientProtocolTest, EnableMultiplayerOfAnotherGameIsNotFound) {
    ASSERT_EQ(client->login("jugador"), LobbyStatus::Ok);
    EnableMultiplayerResponse response = client->enableMultiplayer(MULTI_ID);
    EXPECT_EQ(response.status, LobbyStatus::GameNotFound);
    EXPECT_EQ(response.inviteCode, "");
}

namespace {

// Durante la partida, del otro lado queda el socket ya aceptado. Cada test lo usa como necesita:
// con BaseProtocol para ver los bytes que manda el cliente, o con ServerProtocol para probar que
// el cliente lee lo que manda el servidor de verdad. Cada mensaje entra en el buffer del socket.
class ClientGameProtocolTest: public ::testing::Test {
protected:
    void SetUp() override {
        Socket listener(PORT);
        Socket client_socket(HOST, PORT);
        server_socket.emplace(listener.accept());
        client = std::make_unique<ClientProtocol>(std::move(client_socket));
    }

    std::optional<Socket> server_socket;
    std::unique_ptr<ClientProtocol> client;
};

}  // namespace

TEST_F(ClientGameProtocolTest, SendMoveSendsTagAndDirection) {
    BaseProtocol server(std::move(*server_socket));
    client->sendMove(1, -1);
    EXPECT_EQ(server.recv_u8(), 0x01);
    EXPECT_EQ(server.recv_u8(), 0x01);
    EXPECT_EQ(server.recv_u8(), 0xFF);
}

TEST_F(ClientGameProtocolTest, SendLeaveGameSendsOnlyTheTag) {
    BaseProtocol server(std::move(*server_socket));
    client->sendLeaveGame();
    client->shutdown();
    EXPECT_EQ(server.recv_u8(), 0x02);
    // Despues del tag se corta la conexion: no mando nada mas
    EXPECT_EQ(server.recv_u8(), std::nullopt);
}

TEST_F(ClientGameProtocolTest, RecvsLocationData) {
    ServerProtocol server(std::move(*server_socket));
    ASSERT_TRUE(server.send_location_data(LocationData{"granja"}));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<LocationData>(message));
    EXPECT_EQ(std::get<LocationData>(message).worldId, "granja");
}

TEST_F(ClientGameProtocolTest, RecvsSnapshotWithEveryPlayer) {
    ServerProtocol server(std::move(*server_socket));
    const PlayerState walking{7, "granja", 14500, 3250, -1, 1, 100, 270, 500};
    const PlayerState still{9, "pueblo", 0, 0, 0, 0, 1, 2, 3};
    ASSERT_TRUE(server.send_snapshot(Snapshot{42, {walking, still}, {}}));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<Snapshot>(message));
    const Snapshot& snapshot = std::get<Snapshot>(message);
    EXPECT_EQ(snapshot.tick, 42u);
    ASSERT_EQ(snapshot.players.size(), 2u);
    EXPECT_EQ(snapshot.players[0], walking);
    EXPECT_EQ(snapshot.players[1], still);
}

TEST_F(ClientGameProtocolTest, RecvsSnapshotWithItsEntities) {
    ServerProtocol server(std::move(*server_socket));
    const PlayerState player{7, "granja", 14500, 3250, -1, 1, 100, 270, 500};
    const EntityState as_entity{7, EntityType::Player, "granja", 14500, 3250, -1, 1};
    const EntityState chicken{7, EntityType::Cow, "granja", 2000, 1000, 0, 0, Emote::Ready};
    ASSERT_TRUE(server.send_snapshot(Snapshot{3, {player}, {as_entity, chicken}}));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<Snapshot>(message));
    const Snapshot& snapshot = std::get<Snapshot>(message);
    ASSERT_EQ(snapshot.players.size(), 1u);
    EXPECT_EQ(snapshot.players[0], player);
    EXPECT_EQ(snapshot.entities, (std::vector<EntityState>{as_entity, chicken}));
}

TEST_F(ClientGameProtocolTest, RecvsPlayerInfo) {
    ServerProtocol server(std::move(*server_socket));
    const PlayerInfo info{9, "Martu", APPEARANCE};
    ASSERT_TRUE(server.send_player_info(info));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<PlayerInfo>(message));
    EXPECT_EQ(std::get<PlayerInfo>(message), info);
}

TEST_F(ClientGameProtocolTest, RecvsGameEnded) {
    ServerProtocol server(std::move(*server_socket));
    ASSERT_TRUE(server.send_game_ended(GameEnded{EndReason::Left}));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<GameEnded>(message));
    EXPECT_EQ(std::get<GameEnded>(message).reason, EndReason::Left);
}

TEST_F(ClientGameProtocolTest, UnknownTagThrows) {
    BaseProtocol server(std::move(*server_socket));
    ASSERT_TRUE(server.send_u8(0x7F));
    EXPECT_THROW(client->recvGameMessage(), std::runtime_error);
}


TEST_F(ClientGameProtocolTest, SendSelectSlotSendsTagAndSlot) {
    BaseProtocol server(std::move(*server_socket));
    client->sendSelectSlot(2);
    EXPECT_EQ(server.recv_u8(), 0x03);
    EXPECT_EQ(server.recv_u8(), 0x02);
}

TEST_F(ClientGameProtocolTest, RecvsInventoryState) {
    ServerProtocol server(std::move(*server_socket));
    const InventoryState inventory{2, {{ItemType::Pickaxe, 1}, {ItemType::Empty, 0}}};
    ASSERT_TRUE(server.send_inventory_state(inventory));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<InventoryState>(message));
    EXPECT_EQ(std::get<InventoryState>(message), inventory);
}
TEST_F(ClientGameProtocolTest, SendChatSendsTagAndText) {
    BaseProtocol server(std::move(*server_socket));
    client->sendChat("hola");
    EXPECT_EQ(server.recv_u8(), static_cast<uint8_t>(GameOp::Chat));
    EXPECT_EQ(server.recv_string(), std::optional<std::string>("hola"));
}

TEST_F(ClientGameProtocolTest, RecvsChatMessage) {
    ServerProtocol server(std::move(*server_socket));
    const ChatMessage chat{ChatKind::Public, "Pandamos", "Hola, gente!"};
    ASSERT_TRUE(server.send_chat_message(chat));
    ServerGameMessage message = client->recvGameMessage();
    ASSERT_TRUE(std::holds_alternative<ChatMessage>(message));
    EXPECT_EQ(std::get<ChatMessage>(message), chat);
}

TEST_F(ClientGameProtocolTest, SendInteractSendsOnlyTheTag) {
    BaseProtocol server(std::move(*server_socket));
    client->sendInteract();
    client->shutdown();
    EXPECT_EQ(server.recv_u8(), static_cast<uint8_t>(GameOp::Interact));
    EXPECT_EQ(server.recv_u8(), std::nullopt);
}
