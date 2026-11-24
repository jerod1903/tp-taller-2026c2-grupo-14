#include "server/persistence/toml_saves.h"

#include <cstdint>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "common/map/persistence/TomlTableReader.h"

using stardew::maps::detail::requiredTable;
using stardew::maps::detail::TomlTableReader;

namespace {

template <typename Enum>
int64_t number(Enum value) {
    return static_cast<int64_t>(value);
}

template <typename Enum>
Enum enum_of(const TomlTableReader& table, std::string_view key) {
    return static_cast<Enum>(table.requiredInt(key));
}

float float_of(const TomlTableReader& table, std::string_view key) {
    return static_cast<float>(table.requiredValue<double>(key));
}

bool bool_of(const TomlTableReader& table, std::string_view key) {
    return table.requiredValue<bool>(key);
}

// Recorre las tablas de un array (`[[key]]`), que puede no estar si no hay ninguna.
template <typename Read>
void for_each_table(const TomlTableReader& table, std::string_view key, Read read) {
    if (const toml::array* array = table.optionalArray(key)) {
        for (const toml::node& node: *array) {
            read(TomlTableReader(requiredTable(node, key), std::string(key)));
        }
    }
}

// --- El índice ---

toml::table member_to_toml(const GameMember& member) {
    const CharacterAppearance& look = member.appearance;
    return toml::table{{"username", member.username},
                       {"character", member.characterName},
                       {"hair_style", look.hairStyle},
                       {"hair_color", look.hairColor},
                       {"shirt", look.shirt},
                       {"pants", look.pants},
                       {"hat", look.hat}};
}

GameMember member_from(const TomlTableReader& table) {
    CharacterAppearance look{static_cast<uint16_t>(table.requiredInt("hair_style")),
                             static_cast<uint8_t>(table.requiredInt("hair_color")),
                             static_cast<uint16_t>(table.requiredInt("shirt")),
                             static_cast<uint16_t>(table.requiredInt("pants")),
                             static_cast<uint16_t>(table.requiredInt("hat"))};
    return GameMember{table.requiredValue<std::string>("username"),
                      table.requiredValue<std::string>("character"), look};
}

toml::table record_to_toml(const GameRecord& game) {
    toml::array members;
    for (const GameMember& member: game.members) {
        members.push_back(member_to_toml(member));
    }
    return toml::table{{"id", game.gameId},
                       {"name", game.gameName},
                       {"world", game.worldId},
                       {"multiplayer", game.isMultiplayer},
                       {"max_players", game.maxPlayers},
                       {"invite_code", game.inviteCode},
                       {"members", std::move(members)}};
}

GameRecord record_from(const TomlTableReader& table) {
    GameRecord game{static_cast<uint32_t>(table.requiredInt("id")),
                    table.requiredValue<std::string>("name"),
                    table.requiredValue<std::string>("world"),
                    bool_of(table, "multiplayer"),
                    static_cast<uint8_t>(table.requiredInt("max_players")),
                    {},
                    table.requiredValue<std::string>("invite_code")};
    for_each_table(table, "members", [&game](const TomlTableReader& member) {
        game.members.push_back(member_from(member));
    });
    return game;
}

// --- Una partida ---

// Los slots de un inventario, como `[[slots]] item, quantity, quality`.
toml::array slots_to_toml(const std::vector<InventorySlot>& slots) {
    toml::array array;
    for (const InventorySlot& slot: slots) {
        array.push_back(toml::table{{"item", number(slot.item)},
                                    {"quantity", slot.quantity},
                                    {"quality", number(slot.quality)}});
    }
    return array;
}

std::vector<InventorySlot> slots_from(const TomlTableReader& table) {
    std::vector<InventorySlot> slots;
    for_each_table(table, "slots", [&slots](const TomlTableReader& slot) {
        slots.push_back(InventorySlot{enum_of<ItemType>(slot, "item"),
                                      static_cast<uint8_t>(slot.requiredInt("quantity")),
                                      enum_of<ItemQuality>(slot, "quality")});
    });
    return slots;
}

toml::table player_to_toml(uint32_t player_id, const SavedPlayer& player) {
    return toml::table{{"id", player_id},
                       {"gold", player.gold},
                       {"health", player.health},
                       {"energy", player.energy},
                       {"selected_slot", player.inventory.selectedSlot},
                       {"slots", slots_to_toml(player.inventory.slots)}};
}

SavedPlayer player_from(const TomlTableReader& table) {
    return SavedPlayer{table.requiredInt("gold"), table.requiredInt("health"),
                       table.requiredInt("energy"),
                       InventoryState{static_cast<uint8_t>(table.requiredInt("selected_slot")),
                                      slots_from(table)}};
}

toml::table plot_to_toml(const Position& pos, const SavedPlot& plot) {
    const SavedCrop& crop = plot.crop;
    return toml::table{{"x", pos.x},
                       {"y", pos.y},
                       {"soil", number(plot.soil)},
                       {"crop", number(crop.type)},
                       {"days_grown", crop.days_grown},
                       {"window", crop.window},
                       {"watered", crop.watered},
                       {"window_start", crop.window_start},
                       {"skipped", crop.skipped},
                       {"watering_delay", crop.watering_delay},
                       {"waterings", crop.waterings},
                       {"skipped_total", crop.skipped_total},
                       {"days_ready", crop.days_ready}};
}

SavedPlot plot_from(const TomlTableReader& table) {
    SavedCrop crop{enum_of<CropType>(table, "crop"),   table.requiredInt("days_grown"),
                   table.requiredInt("window"),        bool_of(table, "watered"),
                   float_of(table, "window_start"),    table.requiredInt("skipped"),
                   float_of(table, "watering_delay"),  table.requiredInt("waterings"),
                   table.requiredInt("skipped_total"), table.requiredInt("days_ready")};
    return SavedPlot{enum_of<SoilType>(table, "soil"), crop};
}

Position position_from(const TomlTableReader& table) {
    return Position{table.requiredInt("x"), table.requiredInt("y")};
}

toml::table animal_to_toml(const SavedAnimal& animal) {
    toml::table table{{"kind", number(animal.kind)}, {"x", animal.cell.x},
                      {"y", animal.cell.y},          {"days_left", animal.days_left},
                      {"ready", animal.ready},       {"laid", animal.laid}};
    if (animal.lay_in) {
        table.insert("lay_in", *animal.lay_in);
    }
    return table;
}

SavedAnimal animal_from(const TomlTableReader& table) {
    std::optional<double> lay_in = table.optionalValue<double>("lay_in");
    return SavedAnimal{enum_of<AnimalKind>(table, "kind"),
                       position_from(table),
                       table.requiredInt("days_left"),
                       bool_of(table, "ready"),
                       lay_in ? std::optional<float>(static_cast<float>(*lay_in)) : std::nullopt,
                       bool_of(table, "laid")};
}

toml::table chest_to_toml(const SavedChest& chest) {
    return toml::table{{"area", chest.area},
                       {"x", chest.cell.x},
                       {"y", chest.cell.y},
                       {"slots", slots_to_toml(chest.slots)}};
}

SavedChest chest_from(const TomlTableReader& table) {
    return SavedChest{table.requiredValue<std::string>("area"), position_from(table),
                      slots_from(table)};
}

}  // namespace

toml::table index_to_toml(const std::vector<GameRecord>& games) {
    toml::array records;
    for (const GameRecord& game: games) {
        records.push_back(record_to_toml(game));
    }
    return toml::table{{"games", std::move(records)}};
}

std::vector<GameRecord> index_from_toml(const toml::table& table) {
    std::vector<GameRecord> games;
    for_each_table(TomlTableReader(table, "index"), "games",
                   [&games](const TomlTableReader& game) { games.push_back(record_from(game)); });
    return games;
}

toml::table saved_game_to_toml(const SavedGame& game) {
    toml::array players;
    for (const auto& [player_id, player]: game.players) {
        players.push_back(player_to_toml(player_id, player));
    }
    toml::array plots;
    for (const auto& [pos, plot]: game.farm.plots) {
        plots.push_back(plot_to_toml(pos, plot));
    }
    toml::array items;
    for (const auto& [pos, item]: game.farm.ground_items) {
        items.push_back(toml::table{{"x", pos.x}, {"y", pos.y}, {"item", number(item)}});
    }
    toml::array animals;
    for (const SavedAnimal& animal: game.farm.animals) {
        animals.push_back(animal_to_toml(animal));
    }
    toml::array chests;
    for (const SavedChest& chest: game.farm.chests) {
        chests.push_back(chest_to_toml(chest));
    }
    return toml::table{{"day", game.day},
                       {"players", std::move(players)},
                       {"plots", std::move(plots)},
                       {"ground_items", std::move(items)},
                       {"animals", std::move(animals)},
                       {"chests", std::move(chests)}};
}

SavedGame saved_game_from_toml(const toml::table& table) {
    TomlTableReader game(table, "saved game");
    SavedGame saved{static_cast<uint16_t>(game.requiredInt("day")), {}, {}};
    for_each_table(game, "players", [&saved](const TomlTableReader& player) {
        saved.players[static_cast<uint32_t>(player.requiredInt("id"))] = player_from(player);
    });
    for_each_table(game, "plots", [&saved](const TomlTableReader& plot) {
        saved.farm.plots[position_from(plot)] = plot_from(plot);
    });
    for_each_table(game, "ground_items", [&saved](const TomlTableReader& item) {
        saved.farm.ground_items[position_from(item)] = enum_of<ItemType>(item, "item");
    });
    for_each_table(game, "animals", [&saved](const TomlTableReader& animal) {
        saved.farm.animals.push_back(animal_from(animal));
    });
    // Las partidas guardadas antes de los cofres no tienen "chests": el cofre arranca vacío
    for_each_table(game, "chests", [&saved](const TomlTableReader& chest) {
        saved.farm.chests.push_back(chest_from(chest));
    });
    return saved;
}
