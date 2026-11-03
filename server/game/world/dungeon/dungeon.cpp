#include "server/game/world/dungeon/dungeon.h"

#include <cstddef>
#include <stdexcept>
#include <utility>

Dungeon::Dungeon(std::vector<std::unique_ptr<DungeonLevel>> levels): levels(std::move(levels)) {
    if (this->levels.size() != static_cast<size_t>(LEVEL_COUNT)) {
        throw std::invalid_argument("la mazmorra no tiene LEVEL_COUNT niveles");
    }
}

DungeonLevel& Dungeon::entry_level() { return *levels.front(); }

DungeonLevel& Dungeon::level(int depth) { return *levels.at(depth); }

void Dungeon::update(float dt) {
    for (const std::unique_ptr<DungeonLevel>& dungeon_level: levels) {
        dungeon_level->update(dt);
    }
}
