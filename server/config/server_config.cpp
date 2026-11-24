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
            day_speed};
}

ServerConfig::Player read_player(const toml::table& root) {
    TomlTableReader player = section(root, "player");
    ServerConfig::Player read{
            positive(player.requiredInt("health"), "health"),
            positive(player.requiredInt("energy"), "energy"),
            player.requiredInt("gold"),
            positive(required_float(player, "speed"), "speed"),
            positive(required_float(player, "tired_energy"), "tired_energy"),
            positive(required_float(player, "bad_night_recovery"), "bad_night_recovery"),
            positive(player.requiredInt("sword_damage"), "sword_damage"),
            positive(required_float(player, "attack_cooldown"), "attack_cooldown"),
            positive(required_float(player, "weak_health"), "weak_health"),
            positive(required_float(player, "weak_damage"), "weak_damage"),
            positive(required_float(player, "gift_reach"), "gift_reach")};
    if (read.tired_energy >= 1.0f || read.bad_night_recovery > 1.0f) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": 'tired_energy' and 'bad_night_recovery' are fractions up to 1");
    }
    if (read.weak_health >= 1.0f || read.weak_damage > 1.0f) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": 'weak_health' and 'weak_damage' are fractions up to 1");
    }
    return read;
}

ServerConfig::Inventory read_inventory(const toml::table& root) {
    TomlTableReader inventory = section(root, "inventory");
    int max_stack = positive(inventory.requiredInt("max_stack"), "max_stack");
    if (max_stack > std::numeric_limits<uint8_t>::max()) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'max_stack' is too big");
    }
    return ServerConfig::Inventory{
            static_cast<uint8_t>(max_stack),
            positive(required_float(inventory, "chest_reach"), "chest_reach")};
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
    return ServerConfig::Energy{cost("dig"),     cost("water"), cost("plant"),
                                cost("harvest"), cost("milk"),  cost("pick_up"),
                                cost("attack"),  cost("mine"),  cost("talk")};
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
    ServerConfig::Animals read{animals.requiredInt("cows"),
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
    TomlTableReader vampire = subsection(root, "enemies", "vampire");
    TomlTableReader orc = subsection(root, "enemies", "orc");
    TomlTableReader skull = subsection(root, "enemies", "skull");
    TomlTableReader flame = subsection(root, "enemies", "flame");
    ServerConfig::Enemies read{
            positive(required_float(enemies, "spawn_distance"), "spawn_distance"),
            positive(required_float(enemies, "min_walk"), "min_walk"),
            positive(required_float(enemies, "max_walk"), "max_walk"),
            ServerConfig::Skeleton{
                    positive(skeleton.requiredInt("count"), "count"), read_fighter(skeleton),
                    positive(required_float(skeleton, "revive_seconds"), "revive_seconds")},
            ServerConfig::Vampire{
                    positive(vampire.requiredInt("pairs"), "pairs"), read_fighter(vampire),
                    positive(required_float(vampire, "fast_speed"), "fast_speed"),
                    positive(vampire.requiredInt("strong_damage"), "strong_damage"),
                    positive(required_float(vampire, "revive_seconds"), "revive_seconds"),
                    positive(required_float(vampire, "blood_reach"), "blood_reach")},
            ServerConfig::Orc{positive(orc.requiredInt("min_group"), "min_group"),
                              positive(orc.requiredInt("max_group"), "max_group"),
                              read_fighter(orc)},
            ServerConfig::Skull{positive(required_float(skull, "speed"), "speed"),
                                positive(required_float(skull, "sight"), "sight"),
                                positive(required_float(skull, "cooldown"), "cooldown"),
                                required_float(skull, "green_chance")},
            ServerConfig::Flame{positive(flame.requiredInt("damage"), "damage"),
                                positive(required_float(flame, "speed"), "speed"),
                                positive(required_float(flame, "range"), "range")}};
    if (read.min_walk > read.max_walk) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": [enemies] 'min_walk' is bigger than 'max_walk'");
    }
    if (read.orc.min_group > read.orc.max_group) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": [enemies.orc] 'min_group' is bigger than 'max_group'");
    }
    if (read.skull.green_chance < 0.0f || read.skull.green_chance > 1.0f) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": [enemies.skull] 'green_chance' must be between 0 and 1");
    }
    return read;
}

ServerConfig::Trap read_trap(const TomlTableReader& trap) {
    return ServerConfig::Trap{positive(trap.requiredInt("damage"), "damage"),
                              positive(required_float(trap, "active_time"), "active_time"),
                              positive(required_float(trap, "idle_time"), "idle_time")};
}

ServerConfig::Traps read_traps(const toml::table& root) {
    TomlTableReader traps = section(root, "traps");
    TomlTableReader spikes = subsection(root, "traps", "spikes");
    TomlTableReader flamethrower = subsection(root, "traps", "flamethrower");
    return ServerConfig::Traps{
            positive(traps.requiredInt("spikes_count"), "spikes_count"),
            positive(traps.requiredInt("flamethrowers_count"), "flamethrowers_count"),
            read_trap(spikes),
            ServerConfig::Flamethrower{read_trap(flamethrower),
                                       positive(flamethrower.requiredInt("reach"), "reach")}};
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
    ServerConfig::CropSpec read{
            positive(crop.requiredInt("seed_price"), "seed_price"),
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

ServerConfig::Quality read_quality(const toml::table& root) {
    TomlTableReader quality = subsection(root, "crops", "quality");
    ServerConfig::Quality read{
            required_float(quality, "watering_weight"),
            positive(required_float(quality, "harvest_grace_days"), "harvest_grace_days"),
            positive(required_float(quality, "luck_min"), "luck_min"),
            positive(required_float(quality, "luck_max"), "luck_max"),
            positive(required_float(quality, "good"), "good"),
            positive(required_float(quality, "excellent"), "excellent"),
            positive(required_float(quality, "incredible"), "incredible"),
            positive(required_float(quality, "perfect"), "perfect")};
    if (read.watering_weight < 0.0f || read.watering_weight > 1.0f) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": 'watering_weight' must be between 0 and 1");
    }
    if (read.luck_min > read.luck_max) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'luck_min' is bigger than 'luck_max'");
    }
    if (!(read.good < read.excellent && read.excellent < read.incredible &&
          read.incredible < read.perfect)) {
        throw std::runtime_error(std::string(FILE_NAME) +
                                 ": quality thresholds must go up from 'good' to 'perfect'");
    }
    return read;
}

ServerConfig::Crops read_crops(const toml::table& root) {
    return ServerConfig::Crops{read_crop(root, "strawberry"), read_crop(root, "scallion"),
                               read_crop(root, "potato"), read_crop(root, "pumpkin"),
                               read_quality(root)};
}

ServerConfig::Cheats read_cheats(const toml::table& root) {
    TomlTableReader cheats = section(root, "cheats");
    return ServerConfig::Cheats{positive(cheats.requiredInt("seeds"), "seeds"),
                                positive(cheats.requiredInt("gold"), "gold")};
}

// Lo que paga un comerciante por algo no puede superar lo que lo cobra: si no, comprar y volver a
// vender daría oro infinito.
void check_buyback(int price, int buyback, const std::string& key) {
    if (buyback > price) {
        throw std::runtime_error(std::string(FILE_NAME) + ": '" + key +
                                 "' is bigger than its price");
    }
}

// Necesita los precios de los animales, que se leen en [animals.*].
ServerConfig::Market read_market(const toml::table& root, const ServerConfig::Animals& animals) {
    TomlTableReader market = section(root, "market");
    ServerConfig::Market read{positive(required_float(market, "reach"), "reach"),
                              positive(market.requiredInt("cow_buyback"), "cow_buyback"),
                              positive(market.requiredInt("chicken_buyback"), "chicken_buyback")};
    check_buyback(animals.cow.price, read.cow_buyback, "cow_buyback");
    check_buyback(animals.chicken.price, read.chicken_buyback, "chicken_buyback");
    return read;
}


float chance(const TomlTableReader& reader, const std::string& key) {
    float value = required_float(reader, key);
    if (value < 0.0f || value > 1.0f) {
        throw std::runtime_error(std::string(FILE_NAME) + ": '" + key +
                                 "' is a chance between 0 and 1");
    }
    return value;
}

ServerConfig::OreSpec read_ore(const toml::table& root, const std::string& name) {
    TomlTableReader ore = subsection(root, "ores", name);
    int hits = ore.requiredInt("hits");
    if (hits <= 0) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'ores." + name +
                                 ".hits' must be positive");
    }
    int price = ore.requiredInt("price");
    if (price < 0) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'ores." + name +
                                 ".price' can't be negative");
    }
    return ServerConfig::OreSpec{hits, price};
}

ServerConfig::Mining read_mining(const toml::table& root) {
    TomlTableReader mining = section(root, "mining");
    int ores_per_level = mining.requiredInt("ores_per_level");
    if (ores_per_level < 0) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'ores_per_level' can't be negative");
    }
    int gold_min_depth = mining.requiredInt("gold_min_depth");
    if (gold_min_depth < 0) {
        throw std::runtime_error(std::string(FILE_NAME) + ": 'gold_min_depth' can't be negative");
    }
    return ServerConfig::Mining{ores_per_level,
                                chance(mining, "lapis_break_chance"),
                                chance(mining, "silver_orc_chance"),
                                gold_min_depth,
                                read_ore(root, "coal"),
                                read_ore(root, "lapis"),
                                read_ore(root, "silver"),
                                read_ore(root, "gold")};
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

const ServerConfig::OreSpec& ServerConfig::Mining::of(ItemType ore) const {
    switch (ore) {
        case ItemType::Coal:
            return coal;
        case ItemType::Lapis:
            return lapis;
        case ItemType::Silver:
            return silver;
        case ItemType::GoldOre:
            return gold;
        default:
            break;
    }
    throw std::invalid_argument("not an ore");
}


ServerConfig load_server_config(const std::string& path) {
    toml::table root;
    try {
        root = toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error("could not read " + path + ": " + std::string(e.description()));
    }
    ServerConfig::Animals animals = read_animals(root);
    return ServerConfig{
            read_game(root),   read_player(root),          read_inventory(root), read_energy(root),
            animals,           read_enemies(root),         read_traps(root),     read_crops(root),
            read_cheats(root), read_market(root, animals), read_mining(root)};
}

const ServerConfig& server_config() {
    static const ServerConfig config =
            load_server_config(std::string(TALLER_CONFIG_DIR) + "/" + FILE_NAME);
    return config;
}
