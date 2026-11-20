#include "server/game/day_vote.h"

#include <utility>

DayVote::DayVote(std::set<uint32_t> voters): waiting(std::move(voters)) {}

bool DayVote::sync(const std::set<uint32_t>& connected) {
    auto gone = [&connected](uint32_t player_id) { return connected.count(player_id) == 0; };
    bool changed = std::erase_if(waiting, gone) + std::erase_if(ready, gone) > 0;
    for (uint32_t player_id: connected) {
        if (!ready.count(player_id) && waiting.insert(player_id).second) {
            changed = true;
        }
    }
    return changed;
}

bool DayVote::vote(uint32_t player_id) {
    if (waiting.erase(player_id) == 0) {
        return false;
    }
    ready.insert(player_id);
    return true;
}

bool DayVote::is_done() const { return waiting.empty(); }

uint8_t DayVote::ready_count() const { return static_cast<uint8_t>(ready.size()); }

uint8_t DayVote::total() const { return static_cast<uint8_t>(waiting.size() + ready.size()); }
