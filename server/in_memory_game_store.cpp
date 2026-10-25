#include "in_memory_game_store.h"

#include <algorithm>
#include <iterator>
#include <utility>

InMemoryGameStore::InMemoryGameStore(const std::vector<GameRecord>& games) {
    for (const GameRecord& game: games) {
        this->games.insert_or_assign(game.gameId, game);
    }
}

std::vector<GameRecord> InMemoryGameStore::load_all() {
    std::lock_guard<std::mutex> lock(mutex);
    std::vector<GameRecord> result;
    result.reserve(games.size());
    std::transform(games.begin(), games.end(), std::back_inserter(result),
                   [](const std::pair<const uint32_t, GameRecord>& entry) { return entry.second; });
    return result;
}

void InMemoryGameStore::save(const GameRecord& game) {
    std::lock_guard<std::mutex> lock(mutex);
    games.insert_or_assign(game.gameId, game);
}
