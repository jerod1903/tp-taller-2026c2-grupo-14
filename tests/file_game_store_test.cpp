#include "server/persistence/file_game_store.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <vector>

#include <gtest/gtest.h>

namespace {

// Una partida guardada con un poco de todo
SavedGame sample_game() {
    SavedGame game{4, {}, {}};
    InventoryState inventory{2,
                             {{ItemType::Pickaxe, 1, ItemQuality::Base},
                              {ItemType::Egg, 7, ItemQuality::Good},
                              {ItemType::Empty, 0, ItemQuality::Base}}};
    game.players[0] = SavedPlayer{120, 80, 50, inventory};
    game.players[3] = SavedPlayer{5, 100, 100, {}};
    game.farm.plots[Position{2, 3}] = SavedPlot{
            SoilType::Tilled, SavedCrop{CropType::Pumpkin, 2, 1, true, 120.5f, 0, 3.25f, 4, 1, 0}};
    game.farm.plots[Position{4, 3}] = SavedPlot{SoilType::Dirt, {}};
    game.farm.ground_items[Position{6, 6}] = ItemType::Egg;
    game.farm.animals.push_back(SavedAnimal{AnimalKind::Cow, Position{7, 1}, 1, true, {}, false});
    game.farm.animals.push_back(
            SavedAnimal{AnimalKind::Chicken, Position{8, 2}, 2, false, 33.5f, true});
    return game;
}

GameRecord sample_record(uint32_t id) {
    return GameRecord{id,
                      "granja " + std::to_string(id),
                      "valle",
                      true,
                      4,
                      {{"martu", "Martu", CharacterAppearance{1, 2, 3, 4, 0}},
                       {"jero", "Jero", CharacterAppearance{5, 6, 7, 8, 9}}},
                      "ABC123"};
}

// Cada test en una carpeta propia y vacía
class FileGameStoreTest: public ::testing::Test {
protected:
    void SetUp() override {
        directory = testing::TempDir() + "saves_" +
                    testing::UnitTest::GetInstance()->current_test_info()->name();
        std::filesystem::remove_all(directory);
    }
    void TearDown() override { std::filesystem::remove_all(directory); }

    std::string directory;
};

TEST_F(FileGameStoreTest, AnEmptyDirectoryHasNoGames) {
    FileGameStore store(directory);
    EXPECT_TRUE(store.load_all().empty());
    EXPECT_EQ(store.load_game(1), std::nullopt);
}

TEST_F(FileGameStoreTest, TheIndexSurvivesARestart) {
    {
        FileGameStore store(directory);
        store.save(sample_record(1));
        store.save(sample_record(2));
        GameRecord renamed = sample_record(1);
        renamed.gameName = "otra";
        store.save(renamed);
    }
    FileGameStore store(directory);
    std::vector<GameRecord> games = store.load_all();
    ASSERT_EQ(games.size(), 2u);
    EXPECT_EQ(games[0].gameName, "otra");
    EXPECT_EQ(games[1].members.size(), 2u);
    EXPECT_EQ(games[1].members[1].appearance, (CharacterAppearance{5, 6, 7, 8, 9}));
    EXPECT_EQ(games[1].inviteCode, "ABC123");
}

TEST_F(FileGameStoreTest, AGameIsLoadedAsItWasSaved) {
    {
        FileGameStore store(directory);
        store.save_game(1, sample_game());
    }
    FileGameStore store(directory);
    EXPECT_EQ(store.load_game(1), sample_game());
    EXPECT_EQ(store.load_game(2), std::nullopt);
}

TEST_F(FileGameStoreTest, SavingAgainReplacesTheGame) {
    FileGameStore store(directory);
    store.save_game(1, sample_game());
    SavedGame next = sample_game();
    next.day = 5;
    next.farm.animals.clear();
    store.save_game(1, next);
    EXPECT_EQ(store.load_game(1), next);
}

TEST_F(FileGameStoreTest, ABrokenFileIsReported) {
    std::filesystem::create_directories(directory);
    std::ofstream(directory + "/index.toml") << "games = [ esto no es toml";
    EXPECT_THROW(FileGameStore{directory}, std::runtime_error);
}

}  // namespace
