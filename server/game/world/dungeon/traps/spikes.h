#ifndef SPIKES_H
#define SPIKES_H

#include <cstdint>
#include <vector>

#include "server/game/world/dungeon/traps/trap.h"

// Pinches en el suelo: lastiman a quien esté parado en su celda. Sus valores salen de
// [traps.spikes] en server_config.toml.
class Spikes: public Trap {
public:
    Spikes(uint32_t id, const Position& pos, Location& location,
           const std::vector<Creature*>& targets, float start);

    EntityType entity_type() const override;

protected:
    bool covers(const Position& cell) const override;
};

#endif  // SPIKES_H
