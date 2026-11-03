#include "server/lobby/lobby_session.h"

#include <chrono>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#include <gtest/gtest.h>

#include "common/appearance_catalog.h"
#include "common/base_protocol.h"
#include "common/game_dto.h"
#include "common/lobby_dto.h"
#include "common/logger.h"
#include "common/net/socket.h"
#include "common/operations.h"
#include "server/consts.h"
#include "server/game/game_registry.h"
#include "server/persistence/in_memory_game_store.h"
#include "tests/test_worlds.h"

namespace {

constexpr const char* HOST = "localhost";
constexpr const char* PORT = "18642";

constexpr uint8_t UNKNOWN_OP = 0xFF;
constexpr std::chrono::milliseconds POLL_INTERVAL(10);
constexpr int MAX_POLLS = 200;

constexpr uint8_t MAX_PLAYERS = 4;
const std::vector<WorldInfo> WORLDS = {{"granja", "Granja"}, {"pueblo", "Pueblo"}};
const CharacterAppearance APPEARANCE{3, 2, 5, 1, 4};

// Partida multi precargada en el store de cada test, de otro usuario que el del login por defecto.
constexpr uint32_t MULTI_ID = 7;
constexpr const char* CODE = "ABC234";
constexpr const char* OWNER = "duenio";
const GameRecord MULTI_GAME{
        MULTI_ID, "Multi", "granja", true, MAX_PLAYERS, {{OWNER, "Duenio", APPEARANCE}}, CODE};

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

    void send_login(uint8_t version, const std::string& username) {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(LobbyOp::Login)));
        ASSERT_TRUE(client->send_u8(version));
        ASSERT_TRUE(client->send_string(username));
    }

    void send_login(const std::string& username) { send_login(PROTOCOL_VERSION, username); }

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
            game.worldName = client->recv_string().value_or("");
            game.characterName = client->recv_string().value_or("");
            game.appearance.hairStyle = client->recv_u16().value_or(0);
            game.appearance.hairColor = client->recv_u8().value_or(0);
            game.appearance.shirt = client->recv_u16().value_or(0);
            game.appearance.pants = client->recv_u16().value_or(0);
            game.appearance.hat = client->recv_u16().value_or(0);
            game.isMultiplayer = client->recv_u8().value_or(0) != 0;
            games.push_back(game);
        }
        return games;
    }

    void send_join_game(const std::string& code, const std::string& character_name,
                        const CharacterAppearance& appearance = APPEARANCE) {
        send_op(LobbyOp::JoinGame);
        ASSERT_TRUE(client->send_string(code));
        ASSERT_TRUE(client->send_string(character_name));
        send_appearance(appearance);
    }

    void send_create_game(const std::string& world_id, const std::string& game_name,
                          const std::string& character_name,
                          const CharacterAppearance& appearance = APPEARANCE) {
        send_op(LobbyOp::CreateGame);
        ASSERT_TRUE(client->send_string(world_id));
        ASSERT_TRUE(client->send_string(game_name));
        ASSERT_TRUE(client->send_string(character_name));
        send_appearance(appearance);
    }

    // Lee los ids de un GameEnteredResponse Ok de CREATE_GAME: el gameId lo asigna el registry y
    // el creador es siempre el primer jugador.
    uint32_t recv_created_game_id() {
        std::optional<uint32_t> game_id = client->recv_u32();
        std::optional<uint32_t> player_id = client->recv_u32();
        EXPECT_TRUE(game_id && player_id);
        EXPECT_EQ(player_id.value_or(UINT32_MAX), GameRegistry::OWNER_PLAYER_ID);
        return game_id.value_or(0);
    }

    void send_enable_multiplayer(uint32_t game_id) {
        send_op(LobbyOp::EnableMultiplayer);
        ASSERT_TRUE(client->send_u32(game_id));
    }

    void send_load_game(uint32_t game_id) {
        send_op(LobbyOp::LoadGame);
        ASSERT_TRUE(client->send_u32(game_id));
    }

    void send_appearance(const CharacterAppearance& appearance) {
        ASSERT_TRUE(client->send_u16(appearance.hairStyle));
        ASSERT_TRUE(client->send_u8(appearance.hairColor));
        ASSERT_TRUE(client->send_u16(appearance.shirt));
        ASSERT_TRUE(client->send_u16(appearance.pants));
        ASSERT_TRUE(client->send_u16(appearance.hat));
    }

    // Los ids que siguen al status en un GameEnteredResponse. Van en 0 si el status no es Ok.
    void expect_entered_ids(uint32_t game_id, uint32_t player_id = 0) {
        std::optional<uint32_t> received_game = client->recv_u32();
        std::optional<uint32_t> received_player = client->recv_u32();
        ASSERT_TRUE(received_game && received_player);
        EXPECT_EQ(*received_game, game_id);
        EXPECT_EQ(*received_player, player_id);
    }

    void expect_location(const std::string& world_id) {
        EXPECT_EQ(client->recv_u8(),
                  std::optional<uint8_t>(static_cast<uint8_t>(GameMsg::LocationData)));
        EXPECT_EQ(client->recv_string(), std::optional<std::string>(world_id));
    }

    Snapshot recv_snapshot() {
        EXPECT_EQ(client->recv_u8(),
                  std::optional<uint8_t>(static_cast<uint8_t>(GameMsg::Snapshot)));
        return recv_snapshot_body();
    }

    // Decodifica del lado del cliente lo que manda ServerProtocol::send_snapshot, sin el tag.
    Snapshot recv_snapshot_body() {
        Snapshot snapshot{client->recv_u32().value_or(0), {}};
        uint16_t count = client->recv_u16().value_or(0);
        for (uint16_t i = 0; i < count; ++i) {
            PlayerState player{};
            player.playerId = client->recv_u32().value_or(UINT32_MAX);
            player.areaId = client->recv_string().value_or("");
            player.x = client->recv_u16().value_or(0);
            player.y = client->recv_u16().value_or(0);
            player.dirX = static_cast<int8_t>(client->recv_u8().value_or(0));
            player.dirY = static_cast<int8_t>(client->recv_u8().value_or(0));
            player.progress = client->recv_u16().value_or(0);
            player.health = client->recv_u16().value_or(0);
            player.energy = client->recv_u16().value_or(0);
            player.gold = client->recv_u32().value_or(0);
            snapshot.players.push_back(player);
        }
        return snapshot;
    }

    static std::vector<uint32_t> ids_of(const Snapshot& snapshot) {
        std::vector<uint32_t> ids;
        for (const PlayerState& player: snapshot.players) {
            ids.push_back(player.playerId);
        }
        return ids;
    }

    void send_move(int8_t dir_x, int8_t dir_y) {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(Operation::Move)));
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(dir_x)));
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(dir_y)));
    }

    void send_leave_game() {
        ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(Operation::LeaveGame)));
        ASSERT_TRUE(client->send_u8(0));
        ASSERT_TRUE(client->send_u8(0));
    }

    // Descarta los snapshots que estaban en camino hasta el GameEnded y devuelve su motivo.
    std::optional<EndReason> recv_game_ended() {
        std::optional<uint8_t> tag;
        while ((tag = client->recv_u8()) && *tag == static_cast<uint8_t>(GameMsg::Snapshot)) {
            recv_snapshot_body();
        }
        if (tag != std::optional<uint8_t>(static_cast<uint8_t>(GameMsg::GameEnded))) {
            return std::nullopt;
        }
        std::optional<uint8_t> reason = client->recv_u8();
        return reason ? std::optional<EndReason>(static_cast<EndReason>(*reason)) : std::nullopt;
    }

    void expect_server_closed() { EXPECT_FALSE(client->recv_u8().has_value()); }

    void expect_login_retry_works() {
        send_login("jero");
        expect_status(LobbyStatus::Ok);
    }

    bool wait_session_end() {
        for (int i = 0; i < MAX_POLLS && session->is_alive(); ++i) {
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
        return !session->is_alive();
    }

    // Declarados antes que la sesión: la sesión usa al registry y se destruye primero.
    InMemoryGameStore store{{MULTI_GAME}};
    GameRegistry registry{test_worlds(WORLDS), store, MAX_PLAYERS};
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
    send_login(PROTOCOL_VERSION + 1, "jero");
    expect_status(LobbyStatus::ProtocolMismatch);
    expect_server_closed();
}

TEST_F(LobbySessionTest, VersionIsCheckedBeforeIdentity) {
    send_login(PROTOCOL_VERSION + 1, "");
    expect_status(LobbyStatus::ProtocolMismatch);
    expect_server_closed();
}

TEST_F(LobbySessionTest, EmptyUsernameRespondsInvalidNameAndAllowsRetry) {
    send_login("");
    expect_status(LobbyStatus::InvalidName);
    expect_login_retry_works();
}

TEST_F(LobbySessionTest, TooLongUsernameRespondsInvalidNameAndAllowsRetry) {
    send_login(std::string(ServerConstants::MAX_NAME_LEN + 1, 'a'));
    expect_status(LobbyStatus::InvalidName);
    expect_login_retry_works();
}

TEST_F(LobbySessionTest, UsernameWithSpaceRespondsInvalidNameAndAllowsRetry) {
    send_login("jero 14");
    expect_status(LobbyStatus::InvalidName);
    expect_login_retry_works();
}

TEST_F(LobbySessionTest, UsernameWithNonAsciiRespondsInvalidNameAndAllowsRetry) {
    send_login("ñandú");
    expect_status(LobbyStatus::InvalidName);
    expect_login_retry_works();
}

TEST_F(LobbySessionTest, UnknownOperationClosesWithoutResponse) {
    ASSERT_TRUE(client->send_u8(UNKNOWN_OP));
    expect_server_closed();
}

TEST_F(LobbySessionTest, ClientDisconnectingMidLoginEndsSession) {
    const std::string username = "jero";
    ASSERT_TRUE(client->send_u8(static_cast<uint8_t>(LobbyOp::Login)));
    ASSERT_TRUE(client->send_u8(PROTOCOL_VERSION));
    ASSERT_TRUE(client->send_u16(static_cast<uint16_t>(username.size())));
    ASSERT_TRUE(client->send_bytes(reinterpret_cast<const uint8_t*>(username.data()),
                                   username.size() / 2));
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
    const std::string me = "jero";
    std::optional<uint32_t> mine =
            registry.create_game("Mi granja", "granja", GameMember{me, "Granjero", APPEARANCE});
    registry.create_game("Ajena", "pueblo", GameMember{"otro", "Otro", APPEARANCE});

    send_login(me);
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::ListMyGames);
    std::vector<SavedGameInfo> games = recv_saved_game_list();
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, *mine);
    EXPECT_EQ(games[0].gameName, "Mi granja");
    EXPECT_EQ(games[0].worldName, "Granja");
    EXPECT_EQ(games[0].characterName, "Granjero");
    EXPECT_EQ(games[0].appearance.hairStyle, APPEARANCE.hairStyle);
    EXPECT_EQ(games[0].appearance.hairColor, APPEARANCE.hairColor);
    EXPECT_EQ(games[0].appearance.shirt, APPEARANCE.shirt);
    EXPECT_EQ(games[0].appearance.pants, APPEARANCE.pants);
    EXPECT_EQ(games[0].appearance.hat, APPEARANCE.hat);
    EXPECT_FALSE(games[0].isMultiplayer);
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

TEST_F(LobbySessionTest, CheckInviteCodeOfNonMemberRespondsOnlyOk) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::CheckInviteCode);
    ASSERT_TRUE(client->send_string(CODE));
    expect_status(LobbyStatus::Ok);
    // La respuesta es un solo byte: lo siguiente que llega ya es la próxima operación.
    send_op(LobbyOp::ListWorlds);
    EXPECT_EQ(recv_world_list().size(), WORLDS.size());
}

TEST_F(LobbySessionTest, CheckInviteCodeOfMemberRespondsAlreadyMember) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::CheckInviteCode);
    ASSERT_TRUE(client->send_string(CODE));
    expect_status(LobbyStatus::AlreadyMember);
}

TEST_F(LobbySessionTest, CheckUnknownInviteCodeRespondsInvalidInviteCode) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_op(LobbyOp::CheckInviteCode);
    ASSERT_TRUE(client->send_string("NOEXIS"));
    expect_status(LobbyStatus::InvalidInviteCode);
}

TEST_F(LobbySessionTest, JoinGameByCodeAddsItToMyGames) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_join_game(CODE, "Invitado");
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, 1);
    std::vector<SavedGameInfo> games = registry.games_of("jero");
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, MULTI_ID);
    EXPECT_EQ(games[0].characterName, "Invitado");
    EXPECT_TRUE(games[0].isMultiplayer);
}

TEST_F(LobbySessionTest, JoinGameWithInvalidCharacterNameRespondsInvalidName) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_join_game(CODE, "con espacio");
    expect_status(LobbyStatus::InvalidName);
    expect_entered_ids(0);
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST_F(LobbySessionTest, JoinGameWithUnknownCodeRespondsInvalidInviteCode) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_join_game("NOEXIS", "Invitado");
    expect_status(LobbyStatus::InvalidInviteCode);
    expect_entered_ids(0);
}

TEST_F(LobbySessionTest, LoadedGameKeepsTheUserConnected) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, GameRegistry::OWNER_PLAYER_ID);
    EXPECT_EQ(registry.load_game(MULTI_ID, OWNER).status, LobbyStatus::AlreadyConnected);
}

TEST_F(LobbySessionTest, LoadForeignGameRespondsGameNotFound) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::GameNotFound);
}

TEST_F(LobbySessionTest, ClosingSessionDisconnectsFromItsGame) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    session.reset();
    EXPECT_EQ(registry.load_game(MULTI_ID, OWNER).status, LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, JoinGameBeforeLoginClosesConnection) {
    send_op(LobbyOp::JoinGame);
    expect_server_closed();
}

TEST_F(LobbySessionTest, LoadGameBeforeLoginClosesConnection) {
    send_op(LobbyOp::LoadGame);
    expect_server_closed();
}

TEST_F(LobbySessionTest, CreateGameAddsSinglePlayerGameToMyGames) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_create_game("pueblo", "Granja de prueba", "Granjero");
    expect_status(LobbyStatus::Ok);
    uint32_t game_id = recv_created_game_id();
    EXPECT_NE(game_id, 0u);
    std::vector<SavedGameInfo> games = registry.games_of("jero");
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, game_id);
    EXPECT_EQ(games[0].gameName, "Granja de prueba");
    EXPECT_EQ(games[0].worldName, "Pueblo");
    EXPECT_EQ(games[0].characterName, "Granjero");
    EXPECT_FALSE(games[0].isMultiplayer);
}

TEST_F(LobbySessionTest, CreateGameLeavesCreatorConnected) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_create_game("granja", "Solo", "Granjero");
    expect_status(LobbyStatus::Ok);
    EXPECT_EQ(registry.load_game(recv_created_game_id(), "jero").status,
              LobbyStatus::AlreadyConnected);
}

TEST_F(LobbySessionTest, CreateGameInUnknownWorldRespondsWorldNotFound) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_create_game("../../etc", "Mi granja", "Granjero");
    expect_status(LobbyStatus::WorldNotFound);
    expect_entered_ids(0);
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST_F(LobbySessionTest, CreateGameWithInvalidNamesRespondsInvalidName) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    for (const std::string& game_name:
         {std::string(""), std::string(" Granja"), std::string("Granja "), std::string("Gr\nanja"),
          std::string(ServerConstants::MAX_NAME_LEN + 1, 'a')}) {
        send_create_game("granja", game_name, "Granjero");
        expect_status(LobbyStatus::InvalidName);
        expect_entered_ids(0);
    }
    send_create_game("granja", "Mi granja", "con espacio");
    expect_status(LobbyStatus::InvalidName);
    expect_entered_ids(0);
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST_F(LobbySessionTest, CreateGameBeforeLoginClosesConnection) {
    send_op(LobbyOp::CreateGame);
    expect_server_closed();
}

// Una apariencia con una sola parte fuera del catálogo.
std::vector<CharacterAppearance> invalid_appearances() {
    std::vector<CharacterAppearance> result(5, APPEARANCE);
    result[0].hairStyle = AppearanceCatalog::HAIR_STYLES;
    result[1].hairColor = AppearanceCatalog::HAIR_COLORS;
    result[2].shirt = AppearanceCatalog::SHIRTS;
    result[3].pants = AppearanceCatalog::PANTS;
    result[4].hat = AppearanceCatalog::HATS;
    return result;
}

TEST_F(LobbySessionTest, CreateGameWithAppearanceOutOfCatalogRespondsInvalidAppearance) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    for (const CharacterAppearance& appearance: invalid_appearances()) {
        send_create_game("granja", "Mi granja", "Granjero", appearance);
        expect_status(LobbyStatus::InvalidAppearance);
        expect_entered_ids(0);
    }
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST_F(LobbySessionTest, CreateGameAcceptsLastIdOfEachPart) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_create_game(
            "granja", "Mi granja", "Granjero",
            CharacterAppearance{AppearanceCatalog::HAIR_STYLES - 1,
                                AppearanceCatalog::HAIR_COLORS - 1, AppearanceCatalog::SHIRTS - 1,
                                AppearanceCatalog::PANTS - 1, AppearanceCatalog::HATS - 1});
    expect_status(LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, JoinGameWithAppearanceOutOfCatalogRespondsInvalidAppearance) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    for (const CharacterAppearance& appearance: invalid_appearances()) {
        send_join_game(CODE, "Invitado", appearance);
        expect_status(LobbyStatus::InvalidAppearance);
        expect_entered_ids(0);
    }
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST_F(LobbySessionTest, EnableMultiplayerRespondsInviteCodeThatOthersCanUse) {
    uint32_t game_id = *registry.create_game("Mi granja", "granja", {"jero", "Martu", APPEARANCE});
    registry.leave_game(game_id, "jero");
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_enable_multiplayer(game_id);
    expect_status(LobbyStatus::Ok);
    std::optional<std::string> code = client->recv_string();
    ASSERT_TRUE(code.has_value());
    EXPECT_EQ(code->size(), 6u);

    GameEnteredResponse joined = registry.join_game(*code, GameMember{"otro", "Otro", APPEARANCE});
    EXPECT_EQ(joined.status, LobbyStatus::Ok);
    EXPECT_EQ(joined.gameId, game_id);
}

TEST_F(LobbySessionTest, EnableMultiplayerOfForeignGameRespondsGameNotFound) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_enable_multiplayer(MULTI_ID);
    expect_status(LobbyStatus::GameNotFound);
    EXPECT_EQ(client->recv_string(), std::optional<std::string>(""));
}

TEST_F(LobbySessionTest, EnableMultiplayerByInvitedMemberRespondsNotOwner) {
    ASSERT_EQ(registry.join_game(CODE, GameMember{"jero", "Invitado", APPEARANCE}).status,
              LobbyStatus::Ok);
    registry.leave_game(MULTI_ID, "jero");
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_enable_multiplayer(MULTI_ID);
    expect_status(LobbyStatus::NotOwner);
    EXPECT_EQ(client->recv_string(), std::optional<std::string>(""));
}

TEST_F(LobbySessionTest, EnableMultiplayerBeforeLoginClosesConnection) {
    send_op(LobbyOp::EnableMultiplayer);
    expect_server_closed();
}

TEST_F(LobbySessionTest, EnteredGameSendsTheLocationThenSnapshots) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_create_game("pueblo", "Mi granja", "Martu");
    expect_status(LobbyStatus::Ok);
    recv_created_game_id();
    expect_location("pueblo");
    Snapshot snapshot = recv_snapshot();
    EXPECT_EQ(ids_of(snapshot), (std::vector<uint32_t>{GameRegistry::OWNER_PLAYER_ID}));
    EXPECT_GT(recv_snapshot().tick, snapshot.tick);
}

TEST_F(LobbySessionTest, JoinedPlayerAppearsInTheSnapshotsWithItsId) {
    send_login("jero");
    expect_status(LobbyStatus::Ok);
    send_join_game(CODE, "Invitado");
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, 1);
    expect_location("granja");
    EXPECT_EQ(ids_of(recv_snapshot()), (std::vector<uint32_t>{1}));
}

TEST_F(LobbySessionTest, MovesInGameKeepTheSnapshotsComing) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, GameRegistry::OWNER_PLAYER_ID);
    expect_location("granja");
    uint32_t tick = recv_snapshot().tick;
    send_move(1, 0);
    send_move(0, -1);
    EXPECT_GT(recv_snapshot().tick, tick);
}

TEST_F(LobbySessionTest, DisconnectingInGameEndsTheGame) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, GameRegistry::OWNER_PLAYER_ID);
    expect_location("granja");
    client.reset();
    ASSERT_TRUE(wait_session_end());
    EXPECT_FALSE(registry.is_running(MULTI_ID));
    EXPECT_EQ(registry.load_game(MULTI_ID, OWNER).status, LobbyStatus::Ok);
}

TEST_F(LobbySessionTest, LeaveGameEndsWithGameEndedAndReturnsToTheLobby) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, GameRegistry::OWNER_PLAYER_ID);
    expect_location("granja");
    send_leave_game();
    EXPECT_EQ(recv_game_ended(), std::optional<EndReason>(EndReason::Left));
    EXPECT_FALSE(registry.is_running(MULTI_ID));
    send_op(LobbyOp::ListWorlds);
    EXPECT_EQ(recv_world_list().size(), WORLDS.size());
}

TEST_F(LobbySessionTest, LeftPlayerCanEnterTheGameAgain) {
    send_login(OWNER);
    expect_status(LobbyStatus::Ok);
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, GameRegistry::OWNER_PLAYER_ID);
    expect_location("granja");
    send_leave_game();
    ASSERT_EQ(recv_game_ended(), std::optional<EndReason>(EndReason::Left));
    send_load_game(MULTI_ID);
    expect_status(LobbyStatus::Ok);
    expect_entered_ids(MULTI_ID, GameRegistry::OWNER_PLAYER_ID);
    expect_location("granja");
}

}  // namespace
