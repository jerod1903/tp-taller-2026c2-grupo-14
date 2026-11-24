#include <fstream>
#include <iterator>
#include <stdexcept>
#include <string>

#include <gtest/gtest.h>

#include "client/config/client_config.h"
#include "server/config/server_config.h"

namespace {

std::string write_file(const std::string& name, const std::string& content) {
    std::string path = testing::TempDir() + name;
    std::ofstream(path) << content;
    return path;
}

TEST(ServerConfigTest, TheRepoConfigLoads) {
    const ServerConfig& config = server_config();
    EXPECT_GT(config.game.max_players, 0);
    EXPECT_GT(config.game.tick_rate, 0);
    EXPECT_LE(config.animals.min_walk, config.animals.max_walk);
    EXPECT_LE(config.enemies.min_walk, config.enemies.max_walk);
    EXPECT_LE(config.market.cow_buyback, config.animals.cow.price);
    EXPECT_LE(config.market.chicken_buyback, config.animals.chicken.price);
}

TEST(ServerConfigTest, ABuybackAboveItsPriceIsRejected) {
    std::ifstream repo(std::string(TALLER_CONFIG_DIR) + "/server_config.toml");
    std::string content((std::istreambuf_iterator<char>(repo)), std::istreambuf_iterator<char>());
    std::string line = "cow_buyback = 40";
    ASSERT_NE(content.find(line), std::string::npos);
    content.replace(content.find(line), line.size(), "cow_buyback = 1000");
    EXPECT_THROW(load_server_config(write_file("buyback.toml", content)), std::runtime_error);
}

TEST(ServerConfigTest, AnOrcGroupWithMinAboveMaxIsRejected) {
    std::ifstream repo(std::string(TALLER_CONFIG_DIR) + "/server_config.toml");
    std::string content((std::istreambuf_iterator<char>(repo)), std::istreambuf_iterator<char>());
    std::string line = "min_group = 3";
    ASSERT_NE(content.find(line), std::string::npos);
    content.replace(content.find(line), line.size(), "min_group = 10");
    EXPECT_THROW(load_server_config(write_file("orc_group.toml", content)), std::runtime_error);
}

TEST(ServerConfigTest, AMissingSectionIsRejected) {
    std::string path = write_file("no_animals.toml", R"(
[game]
max_players = 4
tick_rate = 30
outbox_size = 64

[player]
health = 100
energy = 100
gold = 500
speed = 4.0
)");
    EXPECT_THROW(load_server_config(path), std::runtime_error);
}

TEST(ServerConfigTest, ANonPositiveTickRateIsRejected) {
    std::string path = write_file("zero_tick.toml", R"(
[game]
max_players = 4
tick_rate = 0
outbox_size = 64
)");
    EXPECT_THROW(load_server_config(path), std::runtime_error);
}

TEST(ServerConfigTest, AMissingFileIsRejected) {
    EXPECT_THROW(load_server_config(testing::TempDir() + "nope.toml"), std::runtime_error);
}

TEST(ClientConfigTest, TheRepoConfigLoads) {
    const ClientConfig& config = client_config();
    EXPECT_GT(config.ventana.ancho, 0);
    EXPECT_GT(config.ventana.duracion_frame.count(), 0);
    EXPECT_GT(config.audio.canales, 0);
}

TEST(ClientConfigTest, FpsBecomesTheFrameDuration) {
    std::string path = write_file("client.toml", R"(
[window]
width = 800
height = 600
visible_world_height = 200
fps = 50

[audio]
channels = 16
buffer_size = 1024
step_interval_ms = 300
music_volume = 48
)");
    EXPECT_EQ(cargar_client_config(path).ventana.duracion_frame.count(), 20);
}

}  // namespace
