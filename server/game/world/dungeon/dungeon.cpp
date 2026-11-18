#include "server/game/world/dungeon/dungeon.h"

#include <cstddef>
#include <memory>
#include <stdexcept>
#include <utility>
#include <vector>

Dungeon::Dungeon(std::vector<std::unique_ptr<DungeonLevel>> levels): levels(std::move(levels)) {
    if (this->levels.size() != static_cast<size_t>(LEVEL_COUNT)) {
        throw std::invalid_argument("la mazmorra no tiene LEVEL_COUNT niveles");
    }
}

DungeonLevel& Dungeon::entry_level() { return *levels.front(); }

DungeonLevel& Dungeon::level(int depth) { return *levels.at(depth); }

void Dungeon::set_visitors(const std::vector<Creature*>& players) {
    for (const std::unique_ptr<DungeonLevel>& dungeon_level: levels) {
        std::vector<Creature*> here;
        for (Creature* player: players) {
            if (&player->get_location() == dungeon_level.get()) {
                here.push_back(player);
            }
        }
        dungeon_level->set_visitors(std::move(here));
    }
}

DungeonLevel* Dungeon::level_at(const Location& location) {
    for (const std::unique_ptr<DungeonLevel>& dungeon_level: levels) {
        if (dungeon_level.get() == &location) {
            return dungeon_level.get();
        }
    }
    return nullptr;
}

void Dungeon::update(float dt) {
    for (const std::unique_ptr<DungeonLevel>& dungeon_level: levels) {
        dungeon_level->update(dt);
    }
}
