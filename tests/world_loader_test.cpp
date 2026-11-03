#include "server/game/world/world_loader.h"

#include <filesystem>
#include <fstream>
#include <string>
#include <vector>

#include <gtest/gtest.h>

#include "common/logger.h"
#include "common/map/persistence/MapRequirementsLoader.h"

namespace {

namespace fs = std::filesystem;

const fs::path EXAMPLES = fs::path(TALLER_MAPS_DIR) / "examples";

class WorldLoaderTest: public ::testing::Test {
protected:
    void SetUp() override {
        Logger::set_level(LogLevel::Off);
        fs::remove_all(dir);
        fs::create_directories(dir);
    }

    void TearDown() override {
        fs::remove_all(dir);
        Logger::set_level(LogLevel::Info);
    }

    void copy_example(const std::string& example, const std::string& name) {
        fs::copy_file(EXAMPLES / example, dir / name);
    }

    void write(const std::string& name, const std::string& content) {
        std::ofstream(dir / name) << content;
    }

    std::vector<GameWorld> load() {
        return load_worlds(dir.string(), stardew::maps::loadMapRequirements(
                                                 TALLER_MAPS_DIR "/config/map_requirements.toml"));
    }

    fs::path dir = fs::temp_directory_path() / "taller_world_loader_test";
};

TEST_F(WorldLoaderTest, ValidMapIsLoadedWithItsFileNameAsId) {
    copy_example("basic_valid.toml", "granja.toml");
    std::vector<GameWorld> worlds = load();
    ASSERT_EQ(worlds.size(), 1u);
    EXPECT_EQ(worlds[0].info.worldId, "granja");
    EXPECT_EQ(worlds[0].info.name, "Basic Valid World");
    EXPECT_EQ(worlds[0].map.areas.size(), 12u);
}

TEST_F(WorldLoaderTest, MapWithIssuesIsSkipped) {
    copy_example("basic_missing.toml", "incompleto.toml");
    EXPECT_TRUE(load().empty());
}

TEST_F(WorldLoaderTest, UnreadableMapIsSkipped) {
    write("roto.toml", "esto no es [[ toml");
    copy_example("basic_valid.toml", "granja.toml");
    ASSERT_EQ(load().size(), 1u);
}

TEST_F(WorldLoaderTest, OnlyTomlFilesAreRead) {
    copy_example("basic_valid.toml", "granja.txt");
    fs::create_directories(dir / "sub");
    copy_example("basic_valid.toml", "sub/otra.toml");
    EXPECT_TRUE(load().empty());
}

TEST_F(WorldLoaderTest, WorldsAreSortedById) {
    copy_example("basic_valid.toml", "pueblo.toml");
    copy_example("basic_valid.toml", "aldea.toml");
    std::vector<GameWorld> worlds = load();
    ASSERT_EQ(worlds.size(), 2u);
    EXPECT_EQ(worlds[0].info.worldId, "aldea");
    EXPECT_EQ(worlds[1].info.worldId, "pueblo");
}

TEST_F(WorldLoaderTest, MissingDirectoryGivesNoWorlds) {
    fs::remove_all(dir);
    EXPECT_TRUE(load().empty());
}

// Con el mapa de ejemplo el server siempre tiene al menos un mundo para crear partidas.
TEST_F(WorldLoaderTest, TheExampleMapOfTheRepoIsOffered) {
    std::vector<GameWorld> worlds = load_worlds(
            TALLER_MAPS_DIR,
            stardew::maps::loadMapRequirements(TALLER_MAPS_DIR "/config/map_requirements.toml"));
    ASSERT_FALSE(worlds.empty());
    EXPECT_EQ(worlds[0].info.worldId, "valle");
    EXPECT_EQ(worlds[0].info.name, "Valle de ejemplo");
}

}  // namespace
