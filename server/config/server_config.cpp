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
    TomlTableReader time = section(root, "time");
    std::string day_speed_str = time.requiredValue<std::string>("day_speed");
    float day_speed;
    if (day_speed_str == "lento") {
        day_speed = 21 * 60.0f;
    } else if (day_speed_str == "medio") {
        day_speed = 14 * 60.0f;
    } else if (day_speed_str == "rapido") {
        day_speed = 7 * 60.0f;
    } else {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": 'day_speed' must be 'lento', 'medio' or 'rapido'");
    }
    return ServerConfig::Game{
            static_cast<uint8_t>(max_players), positive(game.requiredInt("tick_rate"), "tick_rate"),
            static_cast<unsigned int>(positive(game.requiredInt("outbox_size"), "outbox_size")),
            day_speed };
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

ServerConfig::Energy read_energy(const toml::table& root) {
    TomlTableReader energy = section(root, "energy");
    auto cost = [&energy](const std::string& key) {
        int value = energy.requiredInt(key);
        if (value < 0) {
            throw std::runtime_error(std::string(FILE_NAME) + ": '" + key + "' can't be negative");
        }
        return value;
    };
    return ServerConfig::Energy{cost("dig"),    cost("water"),   cost("plant"),
                                cost("harvest"), cost("milk"),    cost("pick_up"),
                                cost("attack"),  cost("mine"),    cost("talk")};
}

ServerConfig::Species read_species(const toml::table& root, const std::string& name) {
    const toml::table* table = root["animals"][name].as_table();
    if (table == nullptr) {
        throw std::runtime_error(std::string(FILE_NAME) + " is missing section [animals." + name +
                                 "]");
    }
    TomlTableReader species(*table, "[animals." + name + "]");
    return ServerConfig::Species{
            positive(species.requiredInt("price"), "price"),
            positive(species.requiredInt("product_price"), "product_price"),
            positive(species.requiredInt("max_per_farm"), "max_per_farm"),
            positive(species.requiredInt("production_days"), "production_days")};
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
            positive(required_float(animals, "milking_reach"), "milking_reach"),
            read_species(root, "cow"),
            read_species(root, "chicken")};
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

ServerConfig::CropSpec read_crop(const toml::table& root, const std::string& name) {
    TomlTableReader crop = subsection(root, "crops", name);
    auto non_negative = [&crop](const std::string& key) {
        int value = crop.requiredInt(key);
        if (value < 0) {
            throw std::runtime_error(std::string(FILE_NAME) + ": '" + key + "' can't be negative");
        }
        return value;
    };
    ServerConfig::CropSpec read{positive(crop.requiredInt("seed_price"), "seed_price"),
                                positive(crop.requiredInt("base_price"), "base_price"),
                                positive(crop.requiredInt("waterings_per_day"), "waterings_per_day"),
                                positive(crop.requiredInt("days_to_grow"), "days_to_grow"),
                                non_negative("regrow_days"),
                                non_negative("skippable_waterings")};
    if (read.regrow_days >= read.days_to_grow) {
        throw std::runtime_error(std::string(FILE_NAME) + ": [crops." + name +
                                 "] 'regrow_days' must be smaller than 'days_to_grow'");
    }
    return read;
}

ServerConfig::Crops read_crops(const toml::table& root) {
    return ServerConfig::Crops{read_crop(root, "strawberry"), read_crop(root, "scallion"),
                               read_crop(root, "potato"), read_crop(root, "pumpkin")};
}

ServerConfig::Cheats read_cheats(const toml::table& root) {
    TomlTableReader cheats = section(root, "cheats");
    return ServerConfig::Cheats{positive(cheats.requiredInt("seeds"), "seeds"),
                                positive(cheats.requiredInt("gold"), "gold")};
}

}  // namespace

const ServerConfig::CropSpec& ServerConfig::Crops::of(CropType type) const {
    switch (type) {
        case CropType::Strawberry:
            return strawberry;
        case CropType::Scallion:
            return scallion;
        case CropType::Potato:
            return potato;
        case CropType::Pumpkin:
            return pumpkin;
        case CropType::None:
            break;
    }
    throw std::invalid_argument("CropType::None has no spec");
}

ServerConfig load_server_config(const std::string& path) {
    toml::table root;
    try {
        root = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error("could not read " + path + ": " + std::string(e.description()));
    }
    return ServerConfig{read_game(root),    read_player(root),  read_energy(root),
                        read_animals(root), read_enemies(root), read_crops(root),
                        read_cheats(root)};
}

const ServerConfig& server_config() {
    static const ServerConfig config =
            load_server_config(std::string(TALLER_CONFIG_DIR) + "/" + FILE_NAME);
    return config;
}
