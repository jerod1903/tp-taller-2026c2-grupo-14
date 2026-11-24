#include "server/game/interactions/interaction.h"

#include "server/game/interactions/harvest.h"
#include "server/game/interactions/milk.h"
#include "server/game/interactions/open_nearby_chest.h"
#include "server/game/interactions/pick_up.h"

const std::vector<const Interaction*>& interactions() {
    static const OpenNearbyChest open_nearby_chest;
    static const Milk milk;
    static const Harvest harvest;
    static const PickUp pick_up;
    static const std::vector<const Interaction*> all = {&open_nearby_chest, &milk, &harvest,
                                                        &pick_up};
    return all;
}
