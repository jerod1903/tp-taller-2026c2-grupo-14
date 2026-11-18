#include "server/game/interactions/interaction.h"

#include "server/game/interactions/harvest.h"
#include "server/game/interactions/milk.h"
#include "server/game/interactions/pick_up.h"

const std::vector<const Interaction*>& interactions() {
    static const Milk milk;
    static const Harvest harvest;
    static const PickUp pick_up;
    static const std::vector<const Interaction*> all = {&milk, &harvest, &pick_up};
    return all;
}
