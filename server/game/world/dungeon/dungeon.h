#ifndef DUNGEON_H
#define DUNGEON_H

#include <memory>
#include <vector>

#include "server/game/world/dungeon/dungeon_level.h"

class Dungeon {
public:
    static constexpr int LEVEL_COUNT = 10;

    // levels.size() debe ser LEVEL_COUNT, ordenados por profundidad (el índice 0 es la entrada).
    explicit Dungeon(std::vector<std::unique_ptr<DungeonLevel>> levels);

    DungeonLevel& entry_level();

    // Precondición: 0 <= depth < LEVEL_COUNT.
    DungeonLevel& level(int depth);

    void update(float dt);

private:
    std::vector<std::unique_ptr<DungeonLevel>> levels;
};

#endif  // DUNGEON_H
