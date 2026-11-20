#include "server/persistence/file_game_store.h"

#include <filesystem>
#include <fstream>
#include <optional>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include <toml++/toml.hpp>

#include "server/persistence/toml_saves.h"

namespace {
const char* const INDEX_FILE = "index.toml";
const char* const GAME_FILE_PREFIX = "game_";
const char* const FILE_EXTENSION = ".toml";
const char* const TEMPORARY_EXTENSION = ".tmp";

// std::nullopt si el archivo no existe
std::optional<toml::table> read(const std::string& path) {
    if (!std::filesystem::exists(path)) {
        return std::nullopt;
    }
    try {
        return toml::parse_file(path);
    } catch (const toml::parse_error& e) {
        throw std::runtime_error("could not read " + path + ": " + std::string(e.description()));
    }
}

// Escribe en un temporal y lo renombra: el archivo queda viejo o nuevo, nunca a medias
void write(const std::string& path, const toml::table& table) {
    std::string temporary = path + TEMPORARY_EXTENSION;
    {
        std::ofstream output(temporary);
        output << table << '\n';
        if (!output) {
            throw std::runtime_error("could not write " + temporary);
        }
    }
    std::filesystem::rename(temporary, path);
}
}  // namespace

FileGameStore::FileGameStore(std::string directory): directory(std::move(directory)) {
    std::filesystem::create_directories(this->directory);
    if (std::optional<toml::table> index = read(index_path())) {
        for (const GameRecord& game: index_from_toml(*index)) {
            games.insert_or_assign(game.gameId, game);
        }
    }
}

std::vector<GameRecord> FileGameStore::load_all() {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<GameRecord> result;
    for (const auto& [game_id, game]: games) {
        result.push_back(game);
    }
    return result;
}

void FileGameStore::save(const GameRecord& game) {
    std::lock_guard<std::mutex> lock(mutex);
    games.insert_or_assign(game.gameId, game);
    std::vector<GameRecord> all;
    for (const auto& [game_id, record]: games) {
        all.push_back(record);
    }
    write(index_path(), index_to_toml(all));
}

std::optional<SavedGame> FileGameStore::load_game(uint32_t game_id) {
    std::optional<toml::table> table = read(game_path(game_id));
    if (!table) {
        return std::nullopt;
    }
    return saved_game_from_toml(*table);
}

void FileGameStore::save_game(uint32_t game_id, const SavedGame& game) {
    write(game_path(game_id), saved_game_to_toml(game));
}

std::string FileGameStore::index_path() const { return directory + "/" + INDEX_FILE; }

std::string FileGameStore::game_path(uint32_t game_id) const {
    return directory + "/" + GAME_FILE_PREFIX + std::to_string(game_id) + FILE_EXTENSION;
}
