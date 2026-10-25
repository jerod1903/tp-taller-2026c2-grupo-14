#ifndef IN_MEMORY_GAME_STORE_H
#define IN_MEMORY_GAME_STORE_H

#include <cstdint>
#include <map>
#include <mutex>
#include <vector>

#include "game_store.h"

class InMemoryGameStore: public GameStore {
public:
    InMemoryGameStore() = default;
    explicit InMemoryGameStore(const std::vector<GameRecord>& games);

    std::vector<GameRecord> load_all() override;
    void save(const GameRecord& game) override;

private:
    std::map<uint32_t, GameRecord> games;
    std::mutex mutex;
};

#endif  // IN_MEMORY_GAME_STORE_H
