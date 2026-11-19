#include "server/game/world/dungeon/traps/spikes.h"

#include <vector>

Spikes::Spikes(uint32_t id, const Position& pos, Location& location,
               const std::vector<Creature*>& targets, float start):
        Trap(id, pos, location, targets, server_config().traps.spikes, start) {}

EntityType Spikes::entity_type() const { return EntityType::Spikes; }

bool Spikes::covers(const Position& cell) const { return cell == pos; }
