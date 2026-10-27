#include "server/game/game_registry.h"

#include <cstdint>
#include <optional>
#include <set>
#include <string>
#include <thread>
#include <vector>

#include <gtest/gtest.h>

#include "common/lobby_dto.h"
#include "server/persistence/in_memory_game_store.h"
#include "tests/test_helpers.h"

namespace {

constexpr uint8_t MAX_PLAYERS = 4;
constexpr int THREADS = 8;
constexpr int GAMES_PER_THREAD = 20;

const std::vector<WorldInfo> WORLDS = {{"granja", "Granja"}, {"pueblo", "Pueblo"}};
const CharacterAppearance APPEARANCE{3, 2, 10, 1, 0};

GameMember member(const UserId& user_id, const std::string& character) {
    return GameMember{user_id, character, APPEARANCE};
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
    EXPECT_TRUE(registry.games_of(make_user_id()).empty());
}

TEST(GameRegistryTest, CreatedGameAppearsInOwnerGames) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    UserId owner = make_user_id();
    std::optional<uint32_t> id =
            registry.create_game("Mi granja", "granja", true, member(owner, "Martu"));
    ASSERT_TRUE(id.has_value());

    std::vector<SavedGameInfo> games = registry.games_of(owner);
    ASSERT_EQ(games.size(), 1u);
    EXPECT_EQ(games[0].gameId, *id);
    EXPECT_EQ(games[0].gameName, "Mi granja");
    EXPECT_EQ(games[0].characterName, "Martu");
    EXPECT_EQ(games[0].appearance.shirt, APPEARANCE.shirt);
    EXPECT_TRUE(games[0].isMultiplayer);
    EXPECT_EQ(games[0].memberCount, 1);
    EXPECT_EQ(games[0].connectedCount, 0);
    EXPECT_EQ(games[0].maxPlayers, MAX_PLAYERS);
}

TEST(GameRegistryTest, SinglePlayerGameAllowsOnlyItsOwner) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    UserId owner = make_user_id();
    ASSERT_TRUE(registry.create_game("Solo", "granja", false, member(owner, "Martu")));
    std::vector<SavedGameInfo> games = registry.games_of(owner);
    ASSERT_EQ(games.size(), 1u);
    EXPECT_FALSE(games[0].isMultiplayer);
    EXPECT_EQ(games[0].maxPlayers, 1);
}

TEST(GameRegistryTest, CreateGameInUnknownWorldFailsAndSavesNothing) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    UserId owner = make_user_id();
    EXPECT_FALSE(
            registry.create_game("Mi granja", "../../etc/passwd", true, member(owner, "Martu")));
    EXPECT_TRUE(registry.games_of(owner).empty());
    EXPECT_TRUE(store.load_all().empty());
}

TEST(GameRegistryTest, CreatedGameIsSavedInStore) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::optional<uint32_t> id =
            registry.create_game("Mi granja", "pueblo", true, member(make_user_id(), "Martu"));
    std::vector<GameRecord> saved = store.load_all();
    ASSERT_EQ(saved.size(), 1u);
    EXPECT_EQ(saved[0].gameId, *id);
    EXPECT_EQ(saved[0].worldId, "pueblo");
}

TEST(GameRegistryTest, UserOnlySeesOwnGames) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    UserId alice = make_user_id(0x10);
    UserId bob = make_user_id(0x20);
    registry.create_game("De Alice", "granja", true, member(alice, "Ali"));
    registry.create_game("De Bob", "granja", true, member(bob, "Bobby"));
    registry.create_game("Otra de Alice", "pueblo", false, member(alice, "Ali2"));

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
    UserId creator = make_user_id(0x10);
    UserId guest = make_user_id(0x20);
    GameRecord game{7,    "Con amigos", "granja",
                    true, MAX_PLAYERS,  {member(creator, "Creador"), member(guest, "Invitado")}};
    InMemoryGameStore store({game});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);

    std::vector<SavedGameInfo> guest_games = registry.games_of(guest);
    ASSERT_EQ(guest_games.size(), 1u);
    EXPECT_EQ(guest_games[0].gameId, 7u);
    EXPECT_EQ(guest_games[0].characterName, "Invitado");
    EXPECT_EQ(guest_games[0].memberCount, 2);
    EXPECT_EQ(registry.games_of(creator)[0].characterName, "Creador");
}

TEST(GameRegistryTest, NewIdsDoNotCollideWithLoadedGames) {
    GameRecord game{41, "Vieja", "granja", false, 1, {member(make_user_id(0x10), "Old")}};
    InMemoryGameStore store({game});
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::optional<uint32_t> id =
            registry.create_game("Nueva", "granja", false, member(make_user_id(0x20), "New"));
    ASSERT_TRUE(id.has_value());
    EXPECT_GT(*id, 41u);
}

TEST(GameRegistryTest, ConcurrentCreatesAndQueriesStayConsistent) {
    InMemoryGameStore store;
    GameRegistry registry(WORLDS, store, MAX_PLAYERS);
    std::vector<std::vector<uint32_t>> ids(THREADS);
    std::vector<std::thread> threads;
    for (int t = 0; t < THREADS; ++t) {
        threads.emplace_back([&registry, &ids, t]() {
            UserId user = make_user_id(static_cast<uint8_t>(t * 0x10));
            for (int i = 0; i < GAMES_PER_THREAD; ++i) {
                std::optional<uint32_t> id = registry.create_game(
                        "Partida", "granja", true, member(user, "P" + std::to_string(t)));
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
