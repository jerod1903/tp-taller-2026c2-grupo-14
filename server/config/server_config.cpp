#include "server/config/server_config.h"

#include <limits>
#include <stdexcept>
#include <string>

#include <toml++/toml.hpp>

#include "common/map/persistence/TomlTableReader.h"

using stardew::maps::detail::TomlTableReader;

namespace {

const char* const FILE_NAME = "server_config.toml";

TomlTableReader section(const toml::table& root, const std::string& name) {
    const toml::table* table = root[name].as_table();
    if (table == nullptr) {
        throw std::runtime_error(std::string(FILE_NAME) + " is missing section [" + name + "]");
    }
    return TomlTableReader(*table, "[" + name + "]");
}

// Una sección anidada, como [enemies.skeleton]
TomlTableReader subsection(const toml::table& root, const std::string& name,
                           const std::string& child) {
    const toml::table* table = root[name][child].as_table();
    if (table == nullptr) {
        throw std::runtime_error(std::string(FILE_NAME) + " is missing section [" + name + "." +
                                 child + "]");
    }
    return TomlTableReader(*table, "[" + name + "." + child + "]");
}

float required_float(const TomlTableReader& reader, std::string_view key) {
    return static_cast<float>(reader.requiredValue<double>(key));
}

int positive(int value, const std::string& key) {
    if (value <= 0) {
        throw std::runtime_error(std::string(FILE_NAME) + ": '" + key + "' must be positive");
    }
    return value;
}

float positive(float value, const std::string& key) {
    if (value <= 0.0f) {
        throw std::runtime_error(std::string(FILE_NAME) + ": '" + key + "' must be positive");
    }
    return value;
}

ServerConfig::Game read_game(const toml::table& root) {
    TomlTableReader game = section(root, "game");
    int max_players = positive(game.requiredInt("max_players"), "max_players");
    if (max_players > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'max_players' is too big");
    }
    return ServerConfig::Game{
            static_cast<uint8_t>(max_players), positive(game.requiredInt("tick_rate"), "tick_rate"),
            static_cast<unsigned int>(positive(game.requiredInt("outbox_size"), "outbox_size"))};
}

ServerConfig::Player read_player(const toml::table& root) {
    TomlTableReader player = section(root, "player");
    int max_stack = positive(player.requiredInt("max_stack"), "max_stack");
    if (max_stack > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'max_stack' is too big");
    }
    return ServerConfig::Player{
            positive(player.requiredInt("health"), "health"),
            positive(player.requiredInt("energy"), "energy"), player.requiredInt("gold"),
            positive(required_float(player, "speed"), "speed"), static_cast<uint8_t>(max_stack)};
}

ServerConfig::Animals read_animals(const toml::table& root) {
    TomlTableReader animals = section(root, "animals");
    ServerConfig::Animals read{
            animals.requiredInt("cows"),
            animals.requiredInt("chickens"),
            animals.requiredInt("offset_x"),
            animals.requiredInt("offset_y"),
            positive(animals.requiredInt("health"), "health"),
            positive(required_float(animals, "speed"), "speed"),
            positive(required_float(animals, "min_walk"), "min_walk"),
            positive(required_float(animals, "max_walk"), "max_walk"),
            positive(required_float(animals, "cow_ready_seconds"), "cow_ready_seconds"),
            positive(required_float(animals, "milking_reach"), "milking_reach")};
    if (read.min_walk > read.max_walk) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'min_walk' is bigger than 'max_walk'");
    }
    return read;
}

ServerConfig::Fighter read_fighter(const TomlTableReader& enemy) {
    return ServerConfig::Fighter{positive(enemy.requiredInt("health"), "health"),
                                 positive(enemy.requiredInt("damage"), "damage"),
                                 positive(required_float(enemy, "speed"), "speed"),
                                 positive(required_float(enemy, "sight"), "sight"),
                                 positive(required_float(enemy, "reach"), "reach"),
                                 positive(required_float(enemy, "cooldown"), "cooldown")};
}

ServerConfig::Enemies read_enemies(const toml::table& root) {
    TomlTableReader enemies = section(root, "enemies");
    TomlTableReader skeleton = subsection(root, "enemies", "skeleton");
    return ServerConfig::Enemies{
            positive(required_float(enemies, "spawn_distance"), "spawn_distance"),
            ServerConfig::Skeleton{
                    positive(skeleton.requiredInt("count"), "count"), read_fighter(skeleton),
                    positive(required_float(skeleton, "revive_seconds"), "revive_seconds")}};
}

}  // namespace

ServerConfig load_server_config(const std::string& path) {
    toml::table root;
    try {
        root = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error("could not read " + path + ": " + std::string(e.description()));
    }
    return ServerConfig{read_game(root), read_player(root), read_animals(root), read_enemies(root)};
}

const ServerConfig& server_config() {
    static const ServerConfig config =
            load_server_config(std::string(TALLER_CONFIG_DIR) + "/" + FILE_NAME);
    return config;
}
