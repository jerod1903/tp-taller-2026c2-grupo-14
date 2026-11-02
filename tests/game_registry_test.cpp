#include "server/game/game_registry.h"

#include <algorithm>
#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "common/lobby_dto.h"
#include "server/persistence/in_memory_game_store.h"

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
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    const std::vector<WorldInfo>& worlds = registry.list_worlds();
    ASSERT_EQ(worlds.size(), 2u);
    EXPECT_EQ(worlds[0].worldId, "granja");
    EXPECT_EQ(worlds[1].name, "Pueblo");
}

TEST(GameRegistryTest, UnknownUserHasNoGames) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_TRUE(registry.games_of("jero").empty());
}

TEST(GameRegistryTest, CreatedGameAppearsInOwnerGames) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::string owner = "jero";
    std::optional<uint32_t> id =
            registry.create_game("Mi granja", "granja", member(owner, "Granjero"));
    ASSERT_TRUE(id.has_value());

    std::vector<SavedGameInfo> games = registry.games_of(owner);
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, *id);
    EXPECT_EQ(games[0].gameName, "Mi granja");
    EXPECT_EQ(games[0].worldName, "Granja");
    EXPECT_EQ(games[0].characterName, "Granjero");
    EXPECT_EQ(games[0].appearance.shirt, APPEARANCE.shirt);
    EXPECT_FALSE(games[0].isMultiplayer);
}

TEST(GameRegistryTest, CreatorIsConnectedToNewGame) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "Granjero"));
    ASSERT_TRUE(id.has_value());
    EXPECT_EQ(registry.load_game(*id, "jero"), LobbyStatus::AlreadyConnected);
    registry.leave_game(*id, "jero");
    EXPECT_EQ(registry.load_game(*id, "jero"), LobbyStatus::Ok);
}

TEST(GameRegistryTest, NewGamesStartAsSinglePlayerWithoutInviteCode) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    ASSERT_TRUE(registry.create_game("Solo", "granja", member("jero", "Granjero")));
    std::vector<GameRecord> saved = store.load_all();
    ASSERT_EQ(saved.size(), 1u);
    EXPECT_FALSE(saved[0].isMultiplayer);
    EXPECT_EQ(saved[0].maxPlayers, 1);
    EXPECT_TRUE(saved[0].inviteCode.empty());
}

TEST(GameRegistryTest, CreateGameInUnknownWorldFailsAndSavesNothing) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::string owner = "jero";
    EXPECT_FALSE(registry.create_game("Mi granja", "../../etc/passwd", member(owner, "Granjero")));
    EXPECT_TRUE(registry.games_of(owner).empty());
    EXPECT_TRUE(store.load_all().empty());
}

TEST(GameRegistryTest, CreatedGameIsSavedInStore) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::optional<uint32_t> id =
            registry.create_game("Mi granja", "pueblo", member("jero", "Granjero"));
    std::vector<GameRecord> saved = store.load_all();
    ASSERT_EQ(saved.size(), 1u);
    EXPECT_EQ(saved[0].gameId, *id);
    EXPECT_EQ(saved[0].worldId, "pueblo");
}

TEST(GameRegistryTest, UserOnlySeesOwnGames) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);

    std::vector<SavedGameInfo> guest_games = registry.games_of(guest);
    ASSERT_EQ(guest_games.size(), 1u);
    EXPECT_EQ(guest_games[0].gameId, 7u);
    EXPECT_EQ(guest_games[0].characterName, "Invitado");
    EXPECT_EQ(registry.games_of(creator)[0].characterName, "Creador");
}

TEST(GameRegistryTest, NewIdsDoNotCollideWithLoadedGames) {
    GameRecord game{41, "Vieja", "granja", false, 1, {member("user_10", "Old")}, ""};
    InMemoryGameStore store({game});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code("NOEXIS", "user_20"), LobbyStatus::InvalidInviteCode);
}

TEST(GameRegistryTest, CheckInviteCodeOfNonMemberIsOk) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_20"), LobbyStatus::Ok);
}

TEST(GameRegistryTest, CheckInviteCodeOfMemberIsAlreadyMember) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_10"), LobbyStatus::AlreadyMember);
}

// Un miembro de una partida llena tiene que saber que ya está adentro, no que está llena.
TEST(GameRegistryTest, CheckInviteCodeOfMemberOfFullGameIsAlreadyMember) {
    InMemoryGameStore store({multi_game("user_10", 1)});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_10"), LobbyStatus::AlreadyMember);
}

TEST(GameRegistryTest, CheckInviteCodeOfFullGameIsGameFull) {
    InMemoryGameStore store({multi_game("user_10", 1)});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.check_invite_code(CODE, "user_20"), LobbyStatus::GameFull);
}

TEST(GameRegistryTest, JoinAddsMemberAndSavesIt) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::string guest = "user_20";
    JoinGameResponse response = registry.join_game(CODE, member(guest, "Invitado"));
    EXPECT_EQ(response.status, LobbyStatus::Ok);
    EXPECT_EQ(response.gameId, MULTI_ID);
    std::vector<SavedGameInfo> games = registry.games_of(guest);
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].characterName, "Invitado");
    EXPECT_EQ(store.load_all()[0].members.size(), 2u);
}

TEST(GameRegistryTest, JoinWithUnknownCodeIsInvalid) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game("NOEXIS", member("user_20", "Invitado")).status,
              LobbyStatus::InvalidInviteCode);
}

TEST(GameRegistryTest, JoinByMemberIsAlreadyMember) {
    std::string owner = "user_10";
    InMemoryGameStore store({multi_game(owner)});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member(owner, "Otro")).status, LobbyStatus::AlreadyMember);
}

TEST(GameRegistryTest, JoinFullGameIsGameFull) {
    InMemoryGameStore store({multi_game("user_10", 1)});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member("user_20", "Invitado")).status,
              LobbyStatus::GameFull);
}

TEST(GameRegistryTest, JoinWithTakenCharacterNameIsNameTaken) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.join_game(CODE, member("user_20", "Duenio")).status, LobbyStatus::NameTaken);
}

TEST(GameRegistryTest, JoinLeavesTheNewMemberConnected) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::string guest = "user_20";
    ASSERT_EQ(registry.join_game(CODE, member(guest, "Invitado")).status, LobbyStatus::Ok);
    EXPECT_EQ(registry.load_game(MULTI_ID, guest), LobbyStatus::AlreadyConnected);
}

TEST(GameRegistryTest, LoadOwnGameConnectsOnlyOnce) {
    std::string owner = "user_10";
    InMemoryGameStore store({multi_game(owner)});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.load_game(MULTI_ID, owner), LobbyStatus::Ok);
    EXPECT_EQ(registry.load_game(MULTI_ID, owner), LobbyStatus::AlreadyConnected);
    registry.leave_game(MULTI_ID, owner);
    EXPECT_EQ(registry.load_game(MULTI_ID, owner), LobbyStatus::Ok);
}

// Inexistente y ajena responden igual: no se puede averiguar qué ids existen.
TEST(GameRegistryTest, LoadUnknownOrForeignGameIsGameNotFound) {
    InMemoryGameStore store({multi_game("user_10")});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EXPECT_EQ(registry.load_game(MULTI_ID + 1, "user_10"), LobbyStatus::GameNotFound);
    EXPECT_EQ(registry.load_game(MULTI_ID, "user_20"), LobbyStatus::GameNotFound);
}

TEST(GameRegistryTest, EnableMultiplayerTurnsGameIntoMultiWithInviteCode) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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

    JoinGameResponse joined =
            registry.join_game(response.inviteCode, member("invitado", "Invitado"));
    EXPECT_EQ(joined.status, LobbyStatus::Ok);
    EXPECT_EQ(joined.gameId, *id);
}

TEST(GameRegistryTest, EnableMultiplayerTwiceReturnsSameCode) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::optional<uint32_t> id = registry.create_game("Solo", "granja", member("jero", "Granjero"));
    ASSERT_TRUE(id.has_value());
    std::string first = registry.enable_multiplayer(*id, "jero").inviteCode;
    EXPECT_EQ(registry.enable_multiplayer(*id, "jero").inviteCode, first);
}

TEST(GameRegistryTest, EnableMultiplayerOfUnknownOrForeignGameIsGameNotFound) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    EnableMultiplayerResponse response = registry.enable_multiplayer(MULTI_ID, "user_20");
    EXPECT_EQ(response.status, LobbyStatus::NotOwner);
    EXPECT_TRUE(response.inviteCode.empty());
    EXPECT_EQ(registry.enable_multiplayer(MULTI_ID, "user_10").inviteCode, CODE);
}

TEST(GameRegistryTest, InviteCodesAreUniqueAndEasyToType) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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

TEST(GameRegistryTest, ConcurrentCreatesAndQueriesStayConsistent) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
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

}  // namespace
