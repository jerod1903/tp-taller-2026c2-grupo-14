#include "server/game/storage/chest_access.h"

#include <cmath>
#include <iterator>
#include <optional>
#include <vector>

#include "server/config/server_config.h"
#include "server/game/characters/player.h"
#include "server/game/tools/tool.h"

namespace {

constexpr float CELL_CENTER = 0.5f;

bool can_reach(const Player& player, const Location& location, const Position& cell) {
    if (&player.get_location() != &location) {
        return false;
    }
    float dx = player.get_x() - (static_cast<float>(cell.x) + CELL_CENTER);
    float dy = player.get_y() - (static_cast<float>(cell.y) + CELL_CENTER);
    return std::hypot(dx, dy) <= server_config().inventory.chest_reach;
}

}  // namespace

ChestAccess::ChestAccess(PlayerCharacters& characters): characters(characters) {}

std::optional<ChestState> ChestAccess::open(Player& player, const Position& cell) {
    Location& location = player.get_location();
    Chest* chest = location.chest_at(cell);
    if (chest == nullptr || !can_reach(player, location, cell)) {
        return std::nullopt;
    }
    open_chests.insert_or_assign(player.get_id(), OpenChest{&location, cell});
    return chest->state();
}

std::optional<ChestUpdate> ChestAccess::transfer(Player& player, ChestSide from, uint8_t slot) {
    Chest* chest = chest_of(player);
    if (chest == nullptr) {
        return std::nullopt;
    }
    uint8_t moved = 0;
    if (from == ChestSide::Player && is_work_tool(player.item_at(slot))) {
        return std::nullopt;
    }
    if (from == ChestSide::Player) {
        moved = player.give_slot(slot, chest->get_inventory());
    } else if (from == ChestSide::Chest) {
        moved = player.take_slot(chest->get_inventory(), slot);
    }
    if (moved == 0) {
        return std::nullopt;
    }
    return ChestUpdate{chest->state(), viewers_of(open_chests.at(player.get_id()))};
}

std::optional<ChestUpdate> ChestAccess::split(Player& player, ChestSide side, uint8_t slot) {
    Chest* chest = chest_of(player);
    if (chest == nullptr) {
        return std::nullopt;
    }
    bool split = false;
    if (side == ChestSide::Player) {
        split = player.split_slot(slot);
    } else if (side == ChestSide::Chest) {
        split = chest->get_inventory().split_slot(slot);
    }
    if (!split) {
        return std::nullopt;
    }
    return ChestUpdate{chest->state(), viewers_of(open_chests.at(player.get_id()))};
}

void ChestAccess::close(const Player& player) { open_chests.erase(player.get_id()); }

std::vector<uint32_t> ChestAccess::close_unreachable() {
    std::vector<uint32_t> closed;
    for (auto it = open_chests.begin(); it != open_chests.end();) {
        Player* player = characters.find_player(it->first);
        if (player == nullptr) {
            it = open_chests.erase(it);
        } else if (player->is_unconscious() || chest_of(*player) == nullptr) {
            closed.push_back(it->first);
            it = open_chests.erase(it);
        } else {
            ++it;
        }
    }
    return closed;
}

Chest* ChestAccess::chest_of(const Player& player) {
    auto it = open_chests.find(player.get_id());
    if (it == open_chests.end()) {
        return nullptr;
    }
    const OpenChest& open = it->second;
    if (!can_reach(player, *open.location, open.cell)) {
        return nullptr;
    }
    return open.location->chest_at(open.cell);
}

std::vector<uint32_t> ChestAccess::viewers_of(const OpenChest& chest) const {
    std::vector<uint32_t> viewers;
    for (const auto& [player_id, open]: open_chests) {
        if (open == chest) {
            viewers.push_back(player_id);
        }
    }
    return viewers;
}
