#ifndef IN_MEMORY_GAME_STORE_H
#define IN_MEMORY_GAME_STORE_H

#include <cstdint>
#include <map>
#include <mutex>
#include <optional>
#include <vector>

#include "server/persistence/game_store.h"

class InMemoryGameStore: public GameStore {
public:
    InMemoryGameStore() = default;
    explicit InMemoryGameStore(const std::vector<GameRecord>& games);

    std::vector<GameRecord> load_all() override;
    void save(const GameRecord& game) override;
    std::optional<SavedGame> load_game(uint32_t game_id) override;
    void save_game(uint32_t game_id, const SavedGame& game) override;

private:
    std::map<uint32_t, GameRecord> games;
    std::map<uint32_t, SavedGame> states;
    std::mutex mutex;
};

#endif  // IN_MEMORY_GAME_STORE_H
