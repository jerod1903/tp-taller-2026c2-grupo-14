#include "server/game/game_registry.h"

#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <set>
#include <stdexcept>
#include <string>
#include <thread>
#include <variant>
#include <vector>

#include <gtest/gtest.h>

#include "common/lobby_dto.h"
#include "server/game/commands/join_command.h"
#include "server/game/game_event.h"
#include "server/persistence/in_memory_game_store.h"
#include "tests/test_worlds.h"

namespace {

constexpr uint8_t MAX_PLAYERS = 4;
constexpr int THREADS = 8;
constexpr int GAMES_PER_THREAD = 20;
constexpr uint32_t MULTI_ID = 7;
constexpr const char* CODE = "ABC234";

const std::vector<WorldInfo> WORLDS = {{"granja", "Granja"}, {"pueblo", "Pueblo"}};
const CharacterAppearance APPEARANCE{3, 2, 5, 1, 0};

GameMember member(const std::string& username, const std::string& character) {
    return GameMember{username, character, APPEARANCE};
}

TEST(GameRegistryTest, ListWorldsReturnsLoadedWorlds) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    const std::vector<WorldInfo>& worlds = registry.list_worlds();
    ASSERT_EQ(worlds.size(), 2u);
    EXPECT_EQ(worlds[0].worldId, "granja");
    EXPECT_EQ(worlds[1].name, "Pueblo");
}

TEST(GameRegistryTest, UnknownUserHasNoGames) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST(GameRegistryTest, CreatedGameAppearsInOwnerGames) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::string owner = "jero";
    std::optional<uint32_t> id =
            registry.create_game("Mi granja", "granja", member(owner, "Granjero"));
    ASSERT_TRUE(id.has_value());

    std::vector<SavedGameInfo> games = registry.games_of(owner);
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, *id);
    EXPECT_EQ(games[0].gameName, "Mi granja");
    EXPECT_EQ(games[0].worldId, "granja");
    EXPECT_EQ(games[0].worldName, "Granja");
    EXPECT_EQ(games[0].characterName, "Granjero");
    EXPECT_EQ(games[0].appearance.shirt, APPEARANCE.shirt);
    EXPECT_FALSE(games[0].isMultiplayer);
}

TEST(GameRegistryTest, CreatorIsConnectedToNewGame) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "Granjero"));
    ASSERT_TRUE(id.has_value());
    EXPECT_EQ(registry.load_game(*id, "jero").status, LobbyStatus::AlreadyConnected);
    registry.leave_game(*id, "jero");
    EXPECT_EQ(registry.load_game(*id, "jero").status, LobbyStatus::Ok);
}

TEST(GameRegistryTest, NewGamesStartAsSinglePlayerWithoutInviteCode) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    ASSERT_TRUE(registry.create_game("Solo", "granja", member("jero", "Granjero")));
    std::vector<GameRecord> saved = store.load_all();
    ASSERT_EQ(saved.size(), 1u);
    EXPECT_FALSE(saved[0].isMultiplayer);
    EXPECT_EQ(saved[0].maxPlayers, 1);
    EXPECT_TRUE(saved[0].inviteCode.empty());
}

TEST(GameRegistryTest, CreateGameInUnknownWorldFailsAndSavesNothing) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::string owner = "jero";
    EXPECT_FALSE(registry.create_game("Mi granja", "../../etc/passwd", member(owner, "Granjero")));
    EXPECT_TRUE(registry.games_of(owner).empty());
    EXPECT_TRUE(store.load_all().empty());
}

TEST(GameRegistryTest, CreatedGameIsSavedInStore) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id =
            registry.create_game("Mi granja", "pueblo", member("jero", "Granjero"));
    std::vector<GameRecord> saved = store.load_all();
    ASSERT_EQ(saved.size(), 1u);
    EXPECT_EQ(saved[0].gameId, *id);
    EXPECT_EQ(saved[0].worldId, "pueblo");
}

TEST(GameRegistryTest, UserOnlySeesOwnGames) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::string alice = "user_10";
    std::string bob = "user_20";
    registry.create_game("De Alice", "granja", member(alice, "Ali"));
    registry.create_game("De Bob", "granja", member(bob, "Bobby"));
    registry.create_game("Otra de Alice", "pueblo", member(alice, "Ali2"));

    std::vector<SavedGameInfo> alice_games = registry.games_of(alice);
    ASSERT_EQ(alice_games.size(), 2u);
    EXPECT_EQ(alice_games[0].gameName, "De Alice");
    EXPECT_EQ(alice_games[1].gameName, "Otra de Alice");

    std::vector<SavedGameInfo> bob_games = registry.games_of(bob);
    ASSERT_EQ(bob_games.size(), 1u);
    EXPECT_EQ(bob_games[0].gameName, "De Bob");
}

// "Retomar" no depende de que el creador esté conectado: el índice sale del store.
TEST(GameRegistryTest, IndexIsBuiltFromStoreIncludingInvitedMembers) {
    std::string creator = "user_10";
    std::string guest = "user_20";
    GameRecord game{7,       "Con amigos", "granja",
                    true,    MAX_PLAYERS,  {member(creator, "Creador"), member(guest, "Invitado")},
                    "AMIGOS"};
    InMemoryGameStore store({game});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);

    std::vector<SavedGameInfo> guest_games = registry.games_of(guest);
    ASSERT_EQ(guest_games.size(), 1u);
    EXPECT_EQ(guest_games[0].gameId, 7u);
    EXPECT_EQ(guest_games[0].characterName, "Invitado");
    EXPECT_EQ(registry.games_of(creator)[0].characterName, "Creador");
}

TEST(GameRegistryTest, NewIdsDoNotCollideWithLoadedGames) {
    GameRecord game{41, "Vieja", "granja", false, 1, {member("user_10", "Old")}, ""};
    InMemoryGameStore store({game});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Nueva", "granja", member("user_20", "New"));
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(*id, 41u);
}

// Una partida multi ya guardada: hoy toda partida se crea single y todavía no hay operación
// para pasarla a multi.
GameRecord multi_game(const std::string& owner, uint8_t max_players = MAX_PLAYERS) {
    return GameRecord{MULTI_ID, "Multi", "granja", true, max_players, {member(owner, "Duenio")},
                      CODE};
}

TEST(GameRegistryTest, CheckUnknownInviteCodeIsInvalid) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code("NOEXIS", "user_20"), LobbyStatus::InvalidInviteCode);
}

TEST(GameRegistryTest, CheckInviteCodeOfNonMemberIsOk) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_20"), LobbyStatus::Ok);
}

TEST(GameRegistryTest, CheckInviteCodeOfMemberIsAlreadyMember) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_10"), LobbyStatus::AlreadyMember);
}

// Un miembro de una partida llena tiene que saber que ya está adentro, no que está llena.
TEST(GameRegistryTest, CheckInviteCodeOfMemberOfFullGameIsAlreadyMember) {
    InMemoryGameStore store({multi_game("user_10", 1)});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_10"), LobbyStatus::AlreadyMember);
}

TEST(GameRegistryTest, CheckInviteCodeOfFullGameIsGameFull) {
    InMemoryGameStore store({multi_game("user_10", 1)});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_20"), LobbyStatus::GameFull);
}

TEST(GameRegistryTest, JoinAddsMemberAndSavesIt) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::string guest = "user_20";
    GameEnteredResponse response = registry.join_game(CODE, member(guest, "Invitado"));
    EXPECT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.gameId, MULTI_ID);
    std::vector<SavedGameInfo> games = registry.games_of(guest);
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].characterName, "Invitado");
    EXPECT_EQ(store.load_all()[0].members.size(), 2u);
}

TEST(GameRegistryTest, JoinWithUnknownCodeIsInvalid) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game("NOEXIS", member("user_20", "Invitado")).status,
              LobbyStatus::InvalidInviteCode);
}

TEST(GameRegistryTest, JoinByMemberIsAlreadyMember) {
    std::string owner = "user_10";
    InMemoryGameStore store({multi_game(owner)});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member(owner, "Otro")).status, LobbyStatus::AlreadyMember);
}

TEST(GameRegistryTest, JoinFullGameIsGameFull) {
    InMemoryGameStore store({multi_game("user_10", 1)});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member("user_20", "Invitado")).status,
              LobbyStatus::GameFull);
}

TEST(GameRegistryTest, JoinWithTakenCharacterNameIsNameTaken) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member("user_20", "Duenio")).status, LobbyStatus::NameTaken);
}

TEST(GameRegistryTest, JoinLeavesTheNewMemberConnected) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::string guest = "user_20";
    ASSERT_EQ(registry.join_game(CODE, member(guest, "Invitado")).status, LobbyStatus::Ok);
    EXPECT_EQ(registry.load_game(MULTI_ID, guest).status, LobbyStatus::AlreadyConnected);
}

TEST(GameRegistryTest, LoadOwnGameConnectsOnlyOnce) {
    std::string owner = "user_10";
    InMemoryGameStore store({multi_game(owner)});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.load_game(MULTI_ID, owner).status, LobbyStatus::Ok);
    EXPECT_EQ(registry.load_game(MULTI_ID, owner).status, LobbyStatus::AlreadyConnected);
    registry.leave_game(MULTI_ID, owner);
    EXPECT_EQ(registry.load_game(MULTI_ID, owner).status, LobbyStatus::Ok);
}

// El playerId es la posición en members: el mismo al unirse y en cada LOAD_GAME posterior.
TEST(GameRegistryTest, PlayerIdIsStableAcrossJoinAndLoad) {
    std::string owner = "user_10";
    std::string guest = "user_20";
    InMemoryGameStore store({multi_game(owner)});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    GameEnteredResponse joined = registry.join_game(CODE, member(guest, "Invitado"));
    ASSERT_EQ(joined.status, LobbyStatus::Ok);
    EXPECT_EQ(joined.playerId, 1u);
    registry.leave_game(MULTI_ID, guest);

    GameEnteredResponse guest_load = registry.load_game(MULTI_ID, guest);
    ASSERT_EQ(guest_load.status, LobbyStatus::Ok);
    EXPECT_EQ(guest_load.gameId, MULTI_ID);
    EXPECT_EQ(guest_load.playerId, joined.playerId);
    GameEnteredResponse owner_load = registry.load_game(MULTI_ID, owner);
    ASSERT_EQ(owner_load.status, LobbyStatus::Ok);
    EXPECT_EQ(owner_load.playerId, GameRegistry::OWNER_PLAYER_ID);
}

// Inexistente y ajena responden igual: no se puede averiguar qué ids existen.
TEST(GameRegistryTest, LoadUnknownOrForeignGameIsGameNotFound) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.load_game(MULTI_ID + 1, "user_10").status, LobbyStatus::GameNotFound);
    EXPECT_EQ(registry.load_game(MULTI_ID, "user_20").status, LobbyStatus::GameNotFound);
}

TEST(GameRegistryTest, EnableMultiplayerTurnsGameIntoMultiWithInviteCode) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "Granjero"));
    ASSERT_TRUE(id.has_value());
    EnableMultiplayerResponse response = registry.enable_multiplayer(*id, "jero");
    ASSERT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.inviteCode.size(), 6u);

    std::vector<GameRecord> saved = store.load_all();
    ASSERT_EQ(saved.size(), 1u);
    EXPECT_TRUE(saved[0].isMultiplayer);
    EXPECT_EQ(saved[0].maxPlayers, MAX_PLAYERS);
    EXPECT_EQ(saved[0].inviteCode, response.inviteCode);
    EXPECT_TRUE(registry.games_of("jero")[0].isMultiplayer);

    GameEnteredResponse joined =
            registry.join_game(response.inviteCode, member("invitado", "Invitado"));
    EXPECT_EQ(joined.status, LobbyStatus::Ok);
    EXPECT_EQ(joined.gameId, *id);
}

TEST(GameRegistryTest, EnableMultiplayerTwiceReturnsSameCode) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "Granjero"));
    ASSERT_TRUE(id.has_value());
    std::string first = registry.enable_multiplayer(*id, "jero").inviteCode;
    EXPECT_EQ(registry.enable_multiplayer(*id, "jero").inviteCode, first);
}

TEST(GameRegistryTest, EnableMultiplayerOfUnknownOrForeignGameIsGameNotFound) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "Granjero"));
    ASSERT_TRUE(id.has_value());
    EXPECT_EQ(registry.enable_multiplayer(*id + 1, "jero").status, LobbyStatus::GameNotFound);
    EnableMultiplayerResponse foreign = registry.enable_multiplayer(*id, "otro");
    EXPECT_EQ(foreign.status, LobbyStatus::GameNotFound);
    EXPECT_TRUE(foreign.inviteCode.empty());
    EXPECT_FALSE(store.load_all()[0].isMultiplayer);
}

// El primero de members es el creador: un invitado no puede consultar ni generar el código.
TEST(GameRegistryTest, EnableMultiplayerByInvitedMemberIsNotOwner) {
    GameRecord game = multi_game("user_10");
    game.members.push_back(member("user_20", "Invitado"));
    InMemoryGameStore store({game});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EnableMultiplayerResponse response = registry.enable_multiplayer(MULTI_ID, "user_20");
    EXPECT_EQ(response.status, LobbyStatus::NotOwner);
    EXPECT_TRUE(response.inviteCode.empty());
    EXPECT_EQ(registry.enable_multiplayer(MULTI_ID, "user_10").inviteCode, CODE);
}

TEST(GameRegistryTest, InviteCodesAreUniqueAndEasyToType) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::set<std::string> codes;
    for (int i = 0; i < GAMES_PER_THREAD * THREADS; ++i) {
        std::optional<uint32_t> id =
                registry.create_game("Partida", "granja", member("jero", "Granjero"));
        ASSERT_TRUE(id.has_value());
        std::string code = registry.enable_multiplayer(*id, "jero").inviteCode;
        EXPECT_EQ(code.find_first_of("01IO"), std::string::npos);
        codes.insert(code);
    }
    EXPECT_EQ(codes.size(), static_cast<std::size_t>(GAMES_PER_THREAD * THREADS));
}

TEST(GameRegistryTest, ConcurrentJoinsNeverExceedMaxPlayers) {
    InMemoryGameStore store({multi_game("user_F0")});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::vector<LobbyStatus> results(THREADS);
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&registry, &results, t]() {
            results[t] = registry.join_game(CODE, member("user_" + std::to_string(t),
                                                         "P" + std::to_string(t)))
                                 .status;
        });
    }
    for (std::thread& thread: threads) {
        thread.join();
    }
    EXPECT_EQ(std::count(results.begin(), results.end(), LobbyStatus::Ok), MAX_PLAYERS - 1);
    EXPECT_EQ(store.load_all()[0].members.size(), static_cast<std::size_t>(MAX_PLAYERS));
}
// Partida multi de "granja" guardada, con un servidor que ya no carga ese mapa.
GameRecord game_of_removed_world() {
    return GameRecord{MULTI_ID, "Vieja", "granja", true, MAX_PLAYERS, {member("jero", "martin")},
                      CODE};
}

TEST(GameRegistryTest, LoadGameOfARemovedWorldRespondsWorldNotFound) {
    InMemoryGameStore store({game_of_removed_world()});
    GameRegistry registry(test_worlds({{"pueblo", "Pueblo"}}), store, MAX_PLAYERS);
    EXPECT_EQ(registry.load_game(MULTI_ID, "jero").status, LobbyStatus::WorldNotFound);
    EXPECT_EQ(registry.games_of("jero").size(), 1u);
}

TEST(GameRegistryTest, JoinGameOfARemovedWorldRespondsWorldNotFound) {
    InMemoryGameStore store({game_of_removed_world()});
    GameRegistry registry(test_worlds({{"pueblo", "Pueblo"}}), store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member("santi", "Santu")).status,
              LobbyStatus::WorldNotFound);
}

TEST(GameRegistryTest, GameRunsOnlyAfterAttach) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "martin"));
    EXPECT_FALSE(registry.is_running(*id));
    registry.attach(*id);
    EXPECT_TRUE(registry.is_running(*id));
}

TEST(GameRegistryTest, AttachedGameSendsItsWorld) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "pueblo", member("jero", "martin"));
    std::shared_ptr<Outbox> outbox = std::make_shared<Outbox>(64);
    registry.attach(*id).push(std::make_shared<JoinCommand>(0, member("jero", "martin"), outbox));
    GameMessage message = outbox->pop();
    ASSERT_TRUE(std::holds_alternative<LocationData>(message));
    EXPECT_EQ(std::get<LocationData>(message).worldId, "pueblo");
}

TEST(GameRegistryTest, PlayersOfAGameShareItsInbox) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Multi", "granja", member("jero", "martin"));
    std::string code = registry.enable_multiplayer(*id, "jero").inviteCode;
    ASSERT_EQ(registry.join_game(code, member("santi", "Santu")).status, LobbyStatus::Ok);
    EXPECT_EQ(&registry.attach(*id), &registry.attach(*id));
}

TEST(GameRegistryTest, GameStopsWhenTheLastPlayerLeaves) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Multi", "granja", member("jero", "martin"));
    std::string code = registry.enable_multiplayer(*id, "jero").inviteCode;
    registry.join_game(code, member("santi", "Santu"));
    registry.attach(*id);
    registry.leave_game(*id, "santi");
    EXPECT_TRUE(registry.is_running(*id));
    registry.leave_game(*id, "jero");
    EXPECT_FALSE(registry.is_running(*id));
}

TEST(GameRegistryTest, EmptiedGameRelaunchesOnLoad) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "martin"));
    registry.attach(*id);
    registry.leave_game(*id, "jero");
    ASSERT_EQ(registry.load_game(*id, "jero").status, LobbyStatus::Ok);
    std::shared_ptr<Outbox> outbox = std::make_shared<Outbox>(64);
    registry.attach(*id).push(std::make_shared<JoinCommand>(0, member("jero", "martin"), outbox));
    EXPECT_TRUE(registry.is_running(*id));
    EXPECT_TRUE(std::holds_alternative<LocationData>(outbox->pop()));
}

TEST(GameRegistryTest, ConcurrentCreatesAndQueriesStayConsistent) {
    InMemoryGameStore store;
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    std::vector<std::vector<uint32_t>> ids(THREADS);
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&registry, &ids, t]() {
            std::string user = "user_" + std::to_string(t);
            for (int i = 0; i < GAMES_PER_THREAD; ++i) {
                std::optional<uint32_t> id = registry.create_game(
                        "Partida", "granja", member(user, "P" + std::to_string(t)));
                ASSERT_TRUE(id.has_value());
                ids[t].push_back(*id);
                ASSERT_EQ(registry.games_of(user).size(), static_cast<std::size_t>(i + 1));
            }
        });
    }
    for (std::thread& thread: threads) {
        thread.join();
    }
    std::set<uint32_t> unique;
    for (const std::vector<uint32_t>& thread_ids: ids) {
        unique.insert(thread_ids.begin(), thread_ids.end());
    }
    EXPECT_EQ(unique.size(), static_cast<std::size_t>(THREADS * GAMES_PER_THREAD));
    EXPECT_EQ(store.load_all().size(), static_cast<std::size_t>(THREADS * GAMES_PER_THREAD));
}

// --- Partidas guardadas ---

// Un GameStore con lo guardado roto: leerlo falla
class UnreadableStore: public InMemoryGameStore {
public:
    using InMemoryGameStore::InMemoryGameStore;
    std::optional<SavedGame> load_game(uint32_t) override {
        throw std::runtime_error("archivo roto");
    }
};

GameRecord saved_record() {
    return GameRecord{MULTI_ID, "Guardada", "granja", true, MAX_PLAYERS, {member("jero", "martin")},
                      CODE};
}

TEST(GameRegistryTest, ALoadedGameResumesFromWhatWasSaved) {
    InMemoryGameStore store({saved_record()});
    store.save_game(MULTI_ID, SavedGame{5, {}, {}});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    ASSERT_EQ(registry.load_game(MULTI_ID, "jero").status, LobbyStatus::Ok);
    std::shared_ptr<Outbox> outbox = std::make_shared<Outbox>(64);
    registry.attach(MULTI_ID).push(
            std::make_shared<JoinCommand>(0, member("jero", "martin"), outbox));
    GameMessage message = outbox->pop();
    while (!std::holds_alternative<Snapshot>(message)) {
        message = outbox->pop();
    }
    EXPECT_EQ(std::get<Snapshot>(message).day, 5);
}

TEST(GameRegistryTest, AnUnreadableSaveRespondsGameClosedAndDoesNotConnect) {
    UnreadableStore store({saved_record()});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.load_game(MULTI_ID, "jero").status, LobbyStatus::GameClosed);
    // No quedó conectado: al reintentar vuelve a fallar igual, no con AlreadyConnected
    EXPECT_EQ(registry.load_game(MULTI_ID, "jero").status, LobbyStatus::GameClosed);
    EXPECT_FALSE(registry.is_running(MULTI_ID));
}

TEST(GameRegistryTest, JoiningAGameWithAnUnreadableSaveDoesNotAddTheMember) {
    UnreadableStore store({saved_record()});
    GameRegistry registry(test_worlds(WORLDS), store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member("santi", "Santu")).status, LobbyStatus::GameClosed);
    EXPECT_TRUE(registry.games_of("santi").empty());
}

}  // namespace
